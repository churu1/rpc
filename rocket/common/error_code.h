#ifndef ROCKET_COMMON_ERROR_CODE_H
#define ROCKET_COMMON_ERROR_CODE_H

#ifndef SYS_ERROR_PREFIX
#define SYS_ERROR_PREFIX(xx) 1000##xx
#endif

const int ERROR_PEER_CLOSE = SYS_ERROR_PREFIX(0000); // 连接时对端关闭
const int ERROR_FAILED_CONNECT = SYS_ERROR_PREFIX(0001); // 连接失败
const int ERROR_FAILED_GET_REPLY = SYS_ERROR_PREFIX(0002); // 获取回包失败

namespace rocket {

}


#endif