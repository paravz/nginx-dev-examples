
/*
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include <ngx_md5.h>


typedef struct {
    ngx_flag_t  enabled;
} ngx_http_append_loc_conf_t;


typedef struct {
    ngx_md5_t   md5;
} ngx_http_append_ctx_t;


static ngx_int_t ngx_http_append_header_filter(ngx_http_request_t *r);
static ngx_int_t ngx_http_append_body_filter(ngx_http_request_t *r,
    ngx_chain_t *in);
static void *ngx_http_append_create_loc_conf(ngx_conf_t *cf);
static char *ngx_http_append_merge_loc_conf(ngx_conf_t *cf, void *parent,
    void *child);
static ngx_int_t ngx_http_append_init(ngx_conf_t *cf);


static ngx_command_t  ngx_http_append_commands[] = {

    { ngx_string("append"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_FLAG,
      ngx_conf_set_flag_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_append_loc_conf_t, enabled),
      NULL },

      ngx_null_command
};


static ngx_http_module_t  ngx_http_append_module_ctx = {
    NULL,                                  /* preconfiguration */
    ngx_http_append_init,                  /* postconfiguration */

    NULL,                                  /* create main configuration */
    NULL,                                  /* init main configuration */

    NULL,                                  /* create server configuration */
    NULL,                                  /* merge server configuration */

    ngx_http_append_create_loc_conf,       /* create location configuration */
    ngx_http_append_merge_loc_conf         /* merge location configuration */
};


ngx_module_t  ngx_http_append_module = {
    NGX_MODULE_V1,
    &ngx_http_append_module_ctx,           /* module context */
    ngx_http_append_commands,              /* module directives */
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


/* next header and body filters in chain */

static ngx_http_output_header_filter_pt  ngx_http_next_header_filter;
static ngx_http_output_body_filter_pt    ngx_http_next_body_filter;


static ngx_int_t
ngx_http_append_header_filter(ngx_http_request_t *r)
{
    ngx_http_append_loc_conf_t  *plcf;

    ngx_log_debug0(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
                   "http append header handler");

    plcf = ngx_http_get_module_loc_conf(r, ngx_http_append_module);

    if (!plcf->enabled) {
        return ngx_http_next_header_filter(r);
    }

    /* force reading file buffers into memory buffers */
    r->filter_need_in_memory = 1;

    /* reset content length */
    ngx_http_clear_content_length(r);

    /* disable ranges */
    ngx_http_clear_accept_ranges(r);

    /* set weak etag */
    ngx_http_weak_etag(r);

    return ngx_http_next_header_filter(r);
}


static ngx_int_t
ngx_http_append_body_filter(ngx_http_request_t *r, ngx_chain_t *in)
{
    ngx_buf_t                   *b;
    ngx_int_t                    rc;
    ngx_uint_t                   last;
    ngx_chain_t                  out, *cl;
    ngx_http_append_ctx_t       *ctx;
    ngx_http_append_loc_conf_t  *plcf;
    u_char                       md5_buf[16];

    ngx_log_debug0(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
                   "http append body handler");

    plcf = ngx_http_get_module_loc_conf(r, ngx_http_append_module);

    if (!plcf->enabled) {
        return ngx_http_next_body_filter(r, in);
    }

    /* get or create context */

    ctx = ngx_http_get_module_ctx(r, ngx_http_append_module);
    if (ctx == NULL) {
        ctx = ngx_pcalloc(r->pool, sizeof(ngx_http_append_ctx_t));
        if (ctx == NULL) {
            return NGX_ERROR;
        }

        ngx_md5_init(&ctx->md5);

        ngx_http_set_ctx(r, ctx, ngx_http_append_module);
    }

    /* iterate over the buffers and find last_buf */

    last = 0;

    for (cl = in; cl; cl = cl->next) {
        if (cl->buf->last_buf) {
            cl->buf->last_buf = 0;
            cl->buf->sync = 1;
            last = 1;
        }

        ngx_md5_update(&ctx->md5, cl->buf->pos, cl->buf->last - cl->buf->pos);
    }

    rc = ngx_http_next_body_filter(r, in);

    if (rc == NGX_ERROR || !last) {
        return rc;
    }

    /* create second buffer with statistics */

    b = ngx_calloc_buf(r->pool);
    if (b == NULL) {
        return NGX_ERROR;
    }

    b->pos = ngx_pnalloc(r->pool, 32);
    if (b->pos == NULL) {
        return NGX_ERROR;
    }

    ngx_md5_final(md5_buf, &ctx->md5);

    b->last = ngx_hex_dump(b->pos, md5_buf, 16);

    b->temporary = 1;
    b->last_buf = 1;

    out.buf = b;
    out.next = NULL;

    return ngx_http_next_body_filter(r, &out);
}


static void *
ngx_http_append_create_loc_conf(ngx_conf_t *cf)
{
    ngx_http_append_loc_conf_t  *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_append_loc_conf_t));
    if (conf == NULL) {
        return NULL;
    }

    conf->enabled = NGX_CONF_UNSET;

    return conf;
}


static char *
ngx_http_append_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child)
{
    ngx_http_append_loc_conf_t *prev = parent;
    ngx_http_append_loc_conf_t *conf = child;

    ngx_conf_merge_value(conf->enabled, prev->enabled, 0);

    return NGX_CONF_OK;
}


static ngx_int_t
ngx_http_append_init(ngx_conf_t *cf)
{
    /* install handler in header filter chain */

    ngx_http_next_header_filter = ngx_http_top_header_filter;
    ngx_http_top_header_filter = ngx_http_append_header_filter;

    /* install handler in body filter chain */

    ngx_http_next_body_filter = ngx_http_top_body_filter;
    ngx_http_top_body_filter = ngx_http_append_body_filter;

    return NGX_OK;
}
