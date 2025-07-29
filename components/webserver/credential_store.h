
#ifndef ___CREDENTIAL_STORE_H___
#define ___CREDENTIAL_STORE_H___
#define AUTH_NS      "secure"
#define AUTH_KEY     "user_rec"
#define USERNAME_MAX 32
#define SALT_LEN     16
#define HASH_LEN     32
#define PBKDF2_ITERS 100000
#define REGISTERED_USER_FLAG "registered_user"

#include "esp_err.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "mbedtls/md.h"
#include "mbedtls/pkcs5.h"
#include "mbedtls/platform.h"
#include <string.h>
#include <stdbool.h>
typedef struct __attribute__((packed)) {
    uint8_t  ver;             // 1
    uint8_t  algo;            // 1 = PBKDF2-SHA256
    uint32_t iterations;
    uint8_t  salt[SALT_LEN];
    uint8_t  hash[HASH_LEN];
    char     username[USERNAME_MAX];
} auth_rec_t;


int derive_hash(const uint8_t *pwd, size_t pwd_len,
                       const uint8_t *salt, size_t salt_len,
                       uint8_t out[HASH_LEN], uint32_t iters);

bool ct_equal(const uint8_t *a, const uint8_t *b, size_t n);
esp_err_t auth_store_set(const char *username, const char *password);
bool auth_store_check(const char *username, const char *password);
esp_err_t get_user_registered_flag(bool* is_registered);
esp_err_t set_user_registered_flag(void);
#endif // ___CREDENTIAL_STORE_H___
