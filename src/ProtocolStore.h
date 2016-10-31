#ifndef _PROTOCOL_STORE_H_
#define _PROTOCOL_STORE_H_

#include <signal_protocol.h>

#include <string>

namespace signalpp {

namespace ProtocolStore {

void hookSessionStore(signal_protocol_store_context *store_context);
void hookPrekeyStore(signal_protocol_store_context *store_context);
void hookSignedPrekeyStore(signal_protocol_store_context *store_context);
void hookIdentityKeyStore(signal_protocol_store_context *store_context, signal_context *context);
void hookSenderKeyStore(signal_protocol_store_context *store_context, signal_context *context);

void hook(signal_protocol_store_context **_store_context, signal_context *context);

} // namespace ProtocolStore

} // namespace signalpp

#endif // _PROTOCOL_STORE_H_
