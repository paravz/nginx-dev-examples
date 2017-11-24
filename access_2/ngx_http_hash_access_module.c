
/*
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include <ngx_md5.h>


typedef struct {
    ngx_http_complex_value_t  *hash;
    ngx_str_t                  secret;
} ngx_http_hash_access_loc_conf_t;


static ngx_int_t ngx_http_hash_access_handler(ngx_http_request_t *r);
static void *ngx_http_hash_access_create_loc_conf(ngx_conf_t *cf);
static char *ngx_http_hash_access_merge_loc_conf(ngx_conf_t *cf, void *parent,
    void *child);
static ngx_int_t ngx_http_hash_access_init(ngx_conf_t *cf);


static ngx_command_t  ngx_http_hash_access_commands[] = {

    { ngx_string("hash_access"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
      ngx_http_set_complex_value_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_hash_access_loc_conf_t, hash),
      NULL },

    { ngx_string("hash_access_secret"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_str_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_hash_access_loc_conf_t, secret),
      NULL },

      ngx_null_command
};


static ngx_http_module_t  ngx_http_hash_access_module_ctx = {
    NULL,                                  /* preconfiguration */
    ngx_http_hash_access_init,             /* postconfiguration */

    NULL,                                  /* create main configuration */
    NULL,                                  /* init main configuration */

    NULL,                                  /* create server configuration */
    NULL,                                  /* merge server configuration */

    ngx_http_hash_access_create_loc_conf,  /* create location configuration */
    ngx_http_hash_access_merge_loc_conf    /* merge location configuration */
};


ngx_module_t  ngx_http_hash_access_module = {
    NGX_MODULE_V1,
    &ngx_http_hash_access_module_ctx,      /* module context */
    ngx_http_hash_access_commands,         /* module directives */
    NGX_HTTP_MODULE,                       /* module type */
    NULL,                                  /* init master */
    NULL,                                  /* init module */
    NULL,                                  /* init process */
    NULL,                                  /* init thread */
    NULL,                                  /* exit thread */
    NULL,                                  /* exit process */
    NULL,                                  /* exit master */
    NGX_MODULE_V1_PADDING
};


static ngx_int_t
ngx_http_hash_access_handler(ngx_http_request_t *r)
{
    ngx_str_t                         val, hash;
    ngx_md5_t                         md5;
    ngx_http_hash_access_loc_conf_t  *hlcf;
    u_char                            buf[18], md5_buf[16];

    ngx_log_debug0(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
                   "http hash access handler");

    hlcf = ngx_http_get_module_loc_conf(r, ngx_http_hash_access_module);

    if (hlcf->hash == NULL) {
        return NGX_DECLINED;
    }

    /* get user hash value in base64 */

    if (ngx_http_complex_value(r, hlcf->hash, &val) != NGX_OK) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    if (val.len > 24) {
        return NGX_HTTP_FORBIDDEN;
    }

    /* decode user hash value */

    hash.data = buf;

    if (ngx_decode_base64url(&hash, &val) != NGX_OK) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    if (hash.len != 16) {
        return NGX_HTTP_FORBIDDEN;
    }

    /* compute server hash value */

    ngx_md5_init(&md5);
    ngx_md5_update(&md5, r->uri.data, r->uri.len);
    ngx_md5_update(&md5, hlcf->secret.data, hlcf->secret.len);
    ngx_md5_final(md5_buf, &md5);

    /* compare hashes */

    if (ngx_memcmp(buf, md5_buf, 16) != 0) {
        return NGX_HTTP_FORBIDDEN;
    }

    return NGX_OK;
}


static void *
ngx_http_hash_access_create_loc_conf(ngx_conf_t *cf)
{
    ngx_http_hash_access_loc_conf_t  *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_hash_access_loc_conf_t));
    if (conf == NULL) {
        return NULL;
    }

    /*
     * set by ngx_pcalloc():
     *
     *     conf->hash = NULL;
     *     conf->secret = { 0, NULL };
     */

    return conf;
}


static char *
ngx_http_hash_access_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child)
{
    ngx_http_hash_access_loc_conf_t *prev = parent;
    ngx_http_hash_access_loc_conf_t *conf = child;

    ngx_conf_merge_ptr_value(conf->hash, prev->hash, NULL);
    ngx_conf_merge_str_value(conf->secret, prev->secret, "");

    return NGX_CONF_OK;
}


static ngx_int_t
ngx_http_hash_access_init(ngx_conf_t *cf)
{
    ngx_http_handler_pt        *h;
    ngx_http_core_main_conf_t  *cmcf;

    cmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_core_module);

    h = ngx_array_push(&cmcf->phases[NGX_HTTP_ACCESS_PHASE].handlers);
    if (h == NULL) {
        return NGX_ERROR;
    }

    *h = ngx_http_hash_access_handler;

    return NGX_OK;
}
