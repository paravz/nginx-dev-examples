
/*
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>


typedef struct {
    ngx_str_t  name;
} ngx_http_ua_access_loc_conf_t;


static ngx_int_t ngx_http_ua_access_handler(ngx_http_request_t *r);
static void *ngx_http_ua_access_create_loc_conf(ngx_conf_t *cf);
static char *ngx_http_ua_access_merge_loc_conf(ngx_conf_t *cf, void *parent,
    void *child);
static ngx_int_t ngx_http_ua_access_init(ngx_conf_t *cf);


static ngx_command_t  ngx_http_ua_access_commands[] = {

    { ngx_string("ua_access"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_str_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_ua_access_loc_conf_t, name),
      NULL },

      ngx_null_command
};


static ngx_http_module_t  ngx_http_ua_access_module_ctx = {
    NULL,                                  /* preconfiguration */
    ngx_http_ua_access_init,               /* postconfiguration */

    NULL,                                  /* create main configuration */
    NULL,                                  /* init main configuration */

    NULL,                                  /* create server configuration */
    NULL,                                  /* merge server configuration */

    ngx_http_ua_access_create_loc_conf,    /* create location configuration */
    ngx_http_ua_access_merge_loc_conf      /* merge location configuration */
};


ngx_module_t  ngx_http_ua_access_module = {
    NGX_MODULE_V1,
    &ngx_http_ua_access_module_ctx,        /* module context */
    ngx_http_ua_access_commands,           /* module directives */
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
ngx_http_ua_access_handler(ngx_http_request_t *r)
{
    ngx_http_ua_access_loc_conf_t  *ulcf;

    ngx_log_debug0(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
                   "http ua access handler");

    ulcf = ngx_http_get_module_loc_conf(r, ngx_http_ua_access_module);

    if (ulcf->name.len == 0) {
        return NGX_DECLINED;
    }

    if (r->headers_in.user_agent
        && ngx_strstr(r->headers_in.user_agent->value.data, ulcf->name.data))
    {
        return NGX_OK;
    }

    return NGX_HTTP_FORBIDDEN;
}


static void *
ngx_http_ua_access_create_loc_conf(ngx_conf_t *cf)
{
    ngx_http_ua_access_loc_conf_t  *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_ua_access_loc_conf_t));
    if (conf == NULL) {
        return NULL;
    }

    /*
     * set by ngx_pcalloc():
     *
     *     conf->name = { 0, NULL };
     */

    return conf;
}


static char *
ngx_http_ua_access_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child)
{
    ngx_http_ua_access_loc_conf_t *prev = parent;
    ngx_http_ua_access_loc_conf_t *conf = child;

    ngx_conf_merge_str_value(conf->name, prev->name, "");

    return NGX_CONF_OK;
}


static ngx_int_t
ngx_http_ua_access_init(ngx_conf_t *cf)
{
    ngx_http_handler_pt        *h;
    ngx_http_core_main_conf_t  *cmcf;

    cmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_core_module);

    h = ngx_array_push(&cmcf->phases[NGX_HTTP_ACCESS_PHASE].handlers);
    if (h == NULL) {
        return NGX_ERROR;
    }

    *h = ngx_http_ua_access_handler;

    return NGX_OK;
}
