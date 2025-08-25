#include <session.h>
#include "esp_timer.h"
#include <string.h>
#include <stdbool.h>
#include "esp_random.h"
#include "esp_log.h"

static const char *SESSION_TAG = "SESSION";


static session_t g_sessions[MAX_SESSIONS];

static void purge_expired(void)
{
    int64_t now = esp_timer_get_time();
    for (int i = 0; i < MAX_SESSIONS; ++i)
        if (g_sessions[i].used && g_sessions[i].expires_us < now)
            g_sessions[i].used = false;
    ESP_LOGI(SESSION_TAG, "Purged expired sessions");
}
static void rand_token_hex(char *out64)
{
    uint8_t raw[32];
    esp_fill_random(raw, sizeof(raw));
    for (int i = 0; i < 32; ++i) sprintf(&out64[i*2], "%02x", raw[i]);
    out64[64] = '\0';
}

session_t* create_session(const char *user)
{
    purge_expired();
    // Reuse free slot or the oldest one
    int victim = -1;
    int64_t oldest = LLONG_MAX;
    for (int i = 0; i < MAX_SESSIONS; ++i) {
        if (!g_sessions[i].used) { victim = i; break; }
        if (g_sessions[i].expires_us < oldest) { oldest = g_sessions[i].expires_us; victim = i; }
    }
    session_t *s = &g_sessions[victim];
    memset(s, 0, sizeof(*s));
    s->used = true;
    strncpy(s->username, user, USER_MAX-1);
    
    rand_token_hex(s->token);
    s->expires_us = esp_timer_get_time() + SESSION_LIFETIME_US;
    return s;
}

session_t* find_session_by_token(const char *tok)
{
    purge_expired();
    for (int i = 0; i < MAX_SESSIONS; ++i){
        ESP_LOGI(SESSION_TAG,"checking session %d",i);
        if (g_sessions[i].used && strncmp(g_sessions[i].token, tok,TOKEN_LEN) == 0)
            return &g_sessions[i];
    }
    ESP_LOGI(SESSION_TAG,"session not found");
    return NULL;
}

void set_session_cookie(httpd_req_t *req, const session_t *s)
{
    // Add attributes you want: HttpOnly, Secure (if HTTPS), SameSite, Max-Age, Path
    static char cookie[180];
    snprintf(cookie, sizeof(cookie),
             "SID=%s; Path=/; HttpOnly; Max-Age=%d",
             s->token, (int)(SESSION_LIFETIME_US/1000000));
    ESP_LOGI("SESSION", "Setting session cookie: %s", cookie);
    httpd_resp_set_hdr(req, "Set-Cookie", cookie);
}

void remove_session(session_t *s){
    if (s == NULL) {
        ESP_LOGE(SESSION_TAG, "Attempted to remove a null session");
        return; // Handle null session
    }
    s->used = false; // Mark the session as unused
    ESP_LOGI(SESSION_TAG, "Session for user %s removed", s->username);
}

session_t* find_session_by_username(const char *username) {
    for (int i = 0; i < MAX_SESSIONS; ++i) {
        if (g_sessions[i].used && strncmp(g_sessions[i].username, username,USER_MAX) == 0) {
            return &g_sessions[i];
        }
    }
    return NULL;
}

void clear_sessions(void) {
    for (int i = 0; i < MAX_SESSIONS; ++i) {
        g_sessions[i].used = false;
        memset(g_sessions[i].username, 0, USER_MAX);
        memset(g_sessions[i].token, 0, TOKEN_LEN + 1);
        g_sessions[i].expires_us = 0;
    }
    ESP_LOGI("SESSION", "All sessions cleared");
}