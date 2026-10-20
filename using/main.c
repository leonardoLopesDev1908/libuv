#include <uv.h>
#include <stdlib.h>
#include <string.h>

void close_cb(uv_handle_t* handle)
{
    printf("Closing...\n");
}

int end_chat(const uv_buf_t *buf)
{
    char exit[] = "exit\0";
    char *base = buf->base;
    base[4] = '\0';

    return strcmp(base, exit);
}

void fire(uv_timer_t *timer)
{
    printf("Callback function\n");
}

void write_cb(uv_write_t *req, int status) {
    if(status) {
        printf("[ERROR] write_cb: %s\n", uv_strerror(status));
    }

    free(req);
}

void default_alloc_cb(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf)
{   
    buf->base = malloc(suggested_size);
    buf->len = suggested_size;
}

void echo_cb(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf)
{
    if(!end_chat(buf)){
        uv_close((void*)&stream, close_cb);
        free(stream);
      
        return;
    }

    int err;
    uv_write_t *write_t = (uv_write_t*)malloc(sizeof(uv_write_t));

    if((err = uv_write(write_t, stream, buf, 1, write_cb)) != 0)
    {
        printf("ERROR uv_write: %s\n", uv_strerror(err));
        free(write_t);
        exit(1);
    }
}

void client_connected(uv_stream_t* server, int status)
{
    printf("Client connected\n");
    int err;
    if(status < 0)
    {
        printf("ERROR new connection with error: %s\n", uv_strerror(status));
        exit(1);
    }
    
    uv_tcp_t *client = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));
    uv_tcp_init(server->loop, client);

    if((err = uv_accept(server, (uv_stream_t*) client)) != 0)
    {
        printf("ERROR uv_accept: %s\n", uv_strerror(err));
        exit(1);
    }else
    {
        uv_read_start((uv_stream_t*)client, default_alloc_cb, echo_cb);
    }
     
}

int main()
{
    int err;
    printf("uv_version: %u\n", uv_version());

    //LOOP
    uv_loop_t *loop = uv_default_loop();
    err = uv_loop_init(loop);
    if(err != 0){
        printf("Error loop_init\n");
        exit(1);
    }
    
    //TCP
    uv_tcp_t *tcp = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));
    
    err = uv_tcp_init(loop, tcp);
    if(err != 0){
        printf("ERROR: %s\n", uv_strerror(err));
        exit(1);
    }

    struct sockaddr_in addr = {0};
    err = uv_ip4_addr("127.0.0.1", 6969, &addr);
    if(err != 0) 
    {
        printf("ERROR uv_ip4_addr: %s\n", uv_strerror(err));
        exit(1);
    }

    err = uv_tcp_bind(tcp, (void*)&addr, UV_TCP_REUSEPORT);
    if(err != 0) 
    {
        printf("ERROR tcp_bind: %s\n", uv_strerror(err));
        exit(1);
    }

    //LISTEN
    err = uv_listen((void*)tcp, 69, client_connected);
    if(err != 0) 
    {
        printf("ERROR: %s\n", uv_strerror(err));
        exit(1);
    }

    // //TIMER
    // uv_timer_t timer = {0};
    // err = uv_timer_init(loop, &timer);
    // if(err != 0){
    //     printf("Error timer_init: %s\n", uv_strerror(err));
    //     exit(1);
    // }

    // err = uv_timer_start(&timer, fire, 5*1000, 1000);
    // if(err != 0) {
    //     printf("Error timer_start: %s\n", uv_strerror(err));
    //     exit(1);
    // }

    err = uv_run(loop, 0); 
    if(err != 0) 
    {
        printf("Error uv_run: %s\n", uv_strerror(err));
        exit(1);
    }

    uv_loop_close(loop);
    uv_library_shutdown();

    free(loop);
    free(tcp);
    
    return 0;
}
