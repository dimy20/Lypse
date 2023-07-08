#pragma once

#define FMT_LOG_ERROR_RET(err, format, ...) do{   \
    if(err){                                  \
        fprintf(stderr, format, __VA_ARGS__); \
        return false;                         \
    }                                         \
}while(0);

#define LOG_ERROR_RET(err, err_log) do{   \
    if(err){                                  \
        fprintf(stderr, "Error: %s\n", err_log); \
        return false;                         \
    }                                         \
}while(0);
