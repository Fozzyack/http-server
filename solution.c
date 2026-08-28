
// Alternative parser I have found for the request line of http requests

#include <stddef.h>
#include <string.h>

typedef enum {
    HTTP_PARSE_OK,
    HTTP_PARSE_INVALID,
} http_parse_e;

typedef struct {
    char method[8];
    char path[2048];
    char protocol[16];
} http_request;

http_parse_e parse_http_request(char *buff, http_request *request) {
    if (buff == NULL || request == NULL) {
        return HTTP_PARSE_INVALID;
    }

    char *destinations[] = {
        request->method,
        request->path,
        request->protocol,
    };

    size_t capacities[] = {
        sizeof(request->method),
        sizeof(request->path),
        sizeof(request->protocol),
    };

    char *cursor = buff;

    for (size_t field = 0; field < 3; field++) {
        char *start = cursor;

        while (*cursor != '\0' && *cursor != ' ' && *cursor != '\r' && *cursor != '\n') {
            cursor++;
        }

        size_t length = (size_t)(cursor - start);
        if (length == 0 || length >= capacities[field]) {
            return HTTP_PARSE_INVALID;
        }

        memcpy(destinations[field], start, length);
        destinations[field][length] = '\0';

        if (field < 2) {
            if (*cursor != ' ') {
                return HTTP_PARSE_INVALID;
            }

            cursor++;
        }
    }

    if (*cursor != '\0' && *cursor != '\r' && *cursor != '\n') {
        return HTTP_PARSE_INVALID;
    }

    return HTTP_PARSE_OK;
}
