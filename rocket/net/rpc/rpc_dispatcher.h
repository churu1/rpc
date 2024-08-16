#ifndef ROCKET_NET_RPC_RPC_DISPATHCER_H
#define ROCKET_NET_RPC_RPC_DISPATHCER_H

#include <memory>
#include <map>
#include <google/protobuf/service.h>
#include "rocket/net/coder/abstract_protocol.h"

namespace rocket {



class RpcDispathcer {
 public:

  typedef std::shared_ptr<google::protobuf::Service> service_ptr;

  void dispatch(AbstractProtocol::s_ptr request, AbstractProtocol::s_ptr response);

  void registerService(service_ptr);

  void setTinyPBError(std::shared_ptr<TinyPBProtocol> msg, int32_t error_code, const std::string err_info);

 private:
  bool parseServiceFullName(const std::string full_name, std::string& service_name, std::string& method_name);

 private:
  std::map<std::string, service_ptr> m_service_map;
};




















}

#endif