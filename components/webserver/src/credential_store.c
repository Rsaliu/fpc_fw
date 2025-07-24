#include <credential_store.h>
#include "esp_log.h"
#include "mbedtls/platform.h"
#include "mbedtls/pkcs5.h"
#include "esp_random.h"
#include "esp_err.h"
const char *CREDENTIAL_STORE_TAG = "CREDENTIAL_STORE";
int derive_hash(const uint8_t *pwd, size_t pwd_len,
                       const uint8_t *salt, size_t salt_len,
                       uint8_t out[HASH_LEN], uint32_t iters)
{
    return mbedtls_pkcs5_pbkdf2_hmac_ext(MBEDTLS_MD_SHA256, pwd, pwd_len, salt, salt_len,
                                     iters, HASH_LEN, out);
}

bool ct_equal(const uint8_t *a, const uint8_t *b, size_t n)
{
    uint8_t diff = 0;
    for (size_t i = 0; i < n; ++i) diff |= (a[i] ^ b[i]);
    return diff == 0;
}

esp_err_t auth_store_set(const char *username, const char *password)
{
    if (!username || !password) return ESP_ERR_INVALID_ARG;

    auth_rec_t rec = {0};
    rec.ver = 1;
    rec.algo = 1;
    rec.iterations = PBKDF2_ITERS;

    size_t ulen = strnlen(username, USERNAME_MAX);
    if (ulen == USERNAME_MAX) return ESP_ERR_INVALID_ARG; // too long
    memcpy(rec.username, username, ulen + 1);

    // Random salt
    esp_fill_random(rec.salt, SALT_LEN);

    // Hash
    if (derive_hash((const uint8_t*)password, strlen(password),
                    rec.salt, SALT_LEN, rec.hash, rec.iterations) != 0) {
        return ESP_FAIL;
    }

    // Store in NVS
    nvs_handle_t h;
    esp_err_t err = nvs_open(AUTH_NS, NVS_READWRITE, &h);
    if (err != ESP_OK) {
        ESP_LOGE(CREDENTIAL_STORE_TAG, "Failed to open NVS namespace %s: %s", AUTH_NS, esp_err_to_name(err));
        return err;
    }
    err = nvs_set_blob(h, AUTH_KEY, &rec, sizeof(rec));
    if (err == ESP_OK) err = nvs_commit(h);
    nvs_close(h);
    return err;
}

bool auth_store_check(const char *username, const char *password)
{
    if (!username || !password) return false;

    auth_rec_t rec;
    size_t len = sizeof(rec);

    nvs_handle_t h;
    if (nvs_open(AUTH_NS, NVS_READONLY, &h) != ESP_OK) return false;
    esp_err_t err = nvs_get_blob(h, AUTH_KEY, &rec, &len);
    nvs_close(h);
    if (err != ESP_OK || len != sizeof(rec)) return false;

    if (strncmp(username, rec.username, USERNAME_MAX) != 0) return false;

    uint8_t test_hash[HASH_LEN];
    if (derive_hash((const uint8_t*)password, strlen(password),
                    rec.salt, SALT_LEN, test_hash, rec.iterations) != 0) {
        return false;
    }

    bool ok = ct_equal(test_hash, rec.hash, HASH_LEN);
    mbedtls_platform_zeroize(test_hash, sizeof(test_hash));
    return ok;
}
