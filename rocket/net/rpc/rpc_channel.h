#ifndef ROCKET_NET_RPC_RPC_CHANNEL_H
#define ROCKET_NET_RPC_RPC_CHANNEL_H

#include <google/protobuf/service.h>
#include "rocket/net/tcp/net_addr.h"


namespace rocket {

// Abstract interface for an RPC channel.  An RpcChannel represents a
// communication line to a Service which can be used to call that Service's
// methods.  The Service may be running on another machine.  Normally, you
// should not call an RpcChannel directly, but instead construct a stub Service
// wrapping it.  Example:
//   RpcChannel* channel = new MyRpcChannel("remotehost.example.com:1234");
//   MyService* service = new MyService::Stub(channel);
//   service->MyMethod(request, &response, callback);
class RpcChannel : google::protobuf::RpcChannel{
 public:
  RpcChannel(NetAddr::s_ptr peer_addr);
  ~RpcChannel();
  void CallMethod(const google::protobuf::MethodDescriptor* method,
                  google::protobuf::RpcController* controller, const google::protobuf::Message* request,
                  google::protobuf::Message* response, google::protobuf::Closure* done);
 private:
  NetAddr::s_ptr m_peer_addr = nullptr;
  NetAddr::s_ptr m_local_addr = nullptr;
};


}


#endif