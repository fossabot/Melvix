// MIT License, Copyright (c) 2020 Marvin Borner

#ifndef HTTP_H
#define HTTP_H

#include <def.h>
#include <libnet/socket.h>

char *http_data(char *response);
char *http_header_key(char *r, const char *key);
u32 http_content_length(char *r);
char *http_code(char *r);
u32 http_response(const char *http_code, u32 content_length, const char *data, char *resp);
char *http_query_get(const char *url, const char *path);
char *http_query_path(const char *query, char *path);
char *http_receive(struct socket *socket);

#define HTTP_100 "100 Continue"
#define HTTP_101 "101 Switching Protocol"
#define HTTP_102 "102 Processing"
#define HTTP_103 "103 Early Hints"
#define HTTP_200 "200 OK"
#define HTTP_201 "201 Created"
#define HTTP_202 "202 Accepted"
#define HTTP_203 "203 Non-Authoritative Information"
#define HTTP_204 "204 No Content"
#define HTTP_205 "205 Reset Content"
#define HTTP_206 "206 Partial Content"
#define HTTP_207 "207 Multi-Status"
#define HTTP_208 "208 Already Reported"
#define HTTP_226 "226 IM Used"
#define HTTP_300 "300 Multiple Choice"
#define HTTP_301 "301 Moved Permanently"
#define HTTP_302 "302 Found"
#define HTTP_303 "303 See Other"
#define HTTP_304 "304 Not Modified"
#define HTTP_305 "305 Use Proxy"
#define HTTP_306 "306 Unused"
#define HTTP_307 "307 Temporary Redirect"
#define HTTP_308 "308 Permanent Redirect"
#define HTTP_400 "400 Bad Request"
#define HTTP_401 "401 Unauthorized"
#define HTTP_402 "402 Payment Required"
#define HTTP_403 "403 Forbidden"
#define HTTP_404 "404 Not Found"
#define HTTP_405 "405 Method Not Allowed"
#define HTTP_406 "406 Not Acceptable"
#define HTTP_407 "407 Proxy Authentication Required"
#define HTTP_408 "408 Request Timeout"
#define HTTP_409 "409 Conflict"
#define HTTP_410 "410 Gone"
#define HTTP_411 "411 Length Required"
#define HTTP_412 "412 Precondition Failed"
#define HTTP_413 "413 Payload Too Large"
#define HTTP_414 "414 URI Too Long"
#define HTTP_415 "415 Unsupported Media Type"
#define HTTP_416 "416 Range Not Satisfiable"
#define HTTP_417 "417 Expectation Failed"
#define HTTP_418 "418 I'm a teapot"
#define HTTP_421 "421 Misdirected Request"
#define HTTP_422 "422 Unprocessable Entity"
#define HTTP_423 "423 Locked"
#define HTTP_424 "424 Failed Dependency"
#define HTTP_425 "425 Too Early"
#define HTTP_426 "426 Upgrade Required"
#define HTTP_428 "428 Precondition Required"
#define HTTP_429 "429 Too Many Request"
#define HTTP_431 "431 Request Header Fields Too Large"
#define HTTP_451 "451 Unavailable For Legal Reasons"
#define HTTP_500 "500 Internal Server Error"
#define HTTP_501 "501 Not Implemented"
#define HTTP_502 "502 Bad Gateway"
#define HTTP_503 "503 Service Unavailable"
#define HTTP_504 "504 Gateway Timeout"
#define HTTP_505 "505 HTTP Version Not Supported"
#define HTTP_506 "506 Variant Also Negotiates"
#define HTTP_507 "507 Insufficient Storage"
#define HTTP_508 "508 Loop Detected"
#define HTTP_510 "510 Not Extended"
#define HTTP_511 "511 Network Authentication Required"

#endif
