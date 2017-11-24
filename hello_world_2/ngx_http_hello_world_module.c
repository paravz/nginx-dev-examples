
/*
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>


typedef struct {
    ngx_int_t  status;
    ngx_str_t  text;
} ngx_http_hello_world_loc_conf_t;


static ngx_int_t ngx_http_hello_world_handler(ngx_http_request_t *r);
static void *ngx_http_hello_world_create_loc_conf(ngx_conf_t *cf);
static char *ngx_http_hello_world_merge_loc_conf(ngx_conf_t *cf, void *parent,
    void *child);
static char *ngx_http_hello_world(ngx_conf_t *cf, ngx_command_t *cmd,
    void *conf);


static ngx_command_t  ngx_http_hello_world_commands[] = {

    { ngx_string("hello_world"),
      NGX_HTTP_LOC_CONF|NGX_CONF_NOARGS,
      ngx_http_hello_world,
      0,
      0,
      NULL },

    { ngx_string("hello_world_status"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_num_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_hello_world_loc_conf_t, status),
      NULL },

    { ngx_string("hello_world_text"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_str_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_hello_world_loc_conf_t, text),
      NULL },

      ngx_null_command
};


static ngx_http_module_t  ngx_http_hello_world_module_ctx = {
    NULL,                                  /* preconfiguration */
    NULL,                                  /* postconfiguration */

    NULL,                                  /* create main configuration */
    NULL,                                  /* init main configuration */

    NULL,                                  /* create server configuration */
    NULL,                                  /* merge server configuration */

    ngx_http_hello_world_create_loc_conf,  /* create location configuration */
    ngx_http_hello_world_merge_loc_conf    /* merge location configuration */
};


ngx_module_t  ngx_http_hello_world_module = {
    NGX_MODULE_V1,
    &ngx_http_hello_world_module_ctx,      /* module context */
    ngx_http_hello_world_commands,         /* module directives */
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
ngx_http_hello_world_handler(ngx_http_request_t *r)
{
    ngx_buf_t                        *b;
    ngx_int_t                         rc;
    ngx_chain_t                       out;
    ngx_http_hello_world_loc_conf_t  *hlcf;

    ngx_log_debug0(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
                   "http hello_world handler");

    /* ignore client request body if any */

    if (ngx_http_discard_request_body(r) != NGX_OK) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    hlcf = ngx_http_get_module_loc_conf(r, ngx_http_hello_world_module);

    /* send header */

    r->headers_out.status = hlcf->status;
    r->headers_out.content_length_n = hlcf->text.len;

    rc = ngx_http_send_header(r);

    if (rc == NGX_ERROR || rc > NGX_OK || r->header_only) {
        return rc;
    }

    /* send body */

    b = ngx_calloc_buf(r->pool);
    if (b == NULL) {
        return NGX_ERROR;
    }

    b->pos = hlcf->text.data;
    b->last = hlcf->text.data + hlcf->text.len;
    b->memory = hlcf->text.len ? 1 : 0;
    b->last_buf = (r == r->main) ? 1 : 0;
    b->last_in_chain = 1;

    out.buf = b;
    out.next = NULL;

    return ngx_http_output_filter(r, &out);
}


static void *
ngx_http_hello_world_create_loc_conf(ngx_conf_t *cf)
{
    ngx_http_hello_world_loc_conf_t  *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_hello_world_loc_conf_t));
    if (conf == NULL) {
        return NULL;
    }

    /*
     * set by ngx_pcalloc():
     *
     *     conf->text = { 0, NULL };
     */

    conf->status = NGX_CONF_UNSET;

    return conf;
}


static char *
ngx_http_hello_world_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child)
{
    ngx_http_hello_world_loc_conf_t *prev = parent;
    ngx_http_hello_world_loc_conf_t *conf = child;

    ngx_conf_merge_str_value(conf->text, prev->text, "Hello, world!\n");
    ngx_conf_merge_value(conf->status, prev->status, NGX_HTTP_OK);

    return NGX_CONF_OK;
}


static char *
ngx_http_hello_world(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_core_loc_conf_t  *clcf;

    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);

    clcf->handler = ngx_http_hello_world_handler;

    return NGX_CONF_OK;
}
