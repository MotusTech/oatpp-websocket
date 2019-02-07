/***************************************************************************
 *
 * Project         _____    __   ____   _      _
 *                (  _  )  /__\ (_  _)_| |_  _| |_
 *                 )(_)(  /(__)\  )( (_   _)(_   _)
 *                (_____)(__)(__)(__)  |_|    |_|
 *
 *
 * Copyright 2018-present, Leonid Stryzhevskyi, <lganzzzo@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ***************************************************************************/

#ifndef oatpp_websocket_AsyncWebSocket_hpp
#define oatpp_websocket_AsyncWebSocket_hpp

#include "./Frame.hpp"

#include "oatpp/core/data/stream/ChunkedBuffer.hpp"
#include "oatpp/core/async/Coroutine.hpp"

namespace oatpp { namespace websocket {
  
class AsyncWebSocket : public oatpp::base::Controllable {
public:
  typedef oatpp::async::Action Action;
public:
  
  class Listener {
  public:
    typedef oatpp::async::Action Action;
    typedef oatpp::websocket::AsyncWebSocket AsyncWebSocket;
  public:
    
    /**
     * Called when "ping" frame received
     */
    virtual Action onPing(oatpp::async::AbstractCoroutine* parentCoroutine,
                          const Action& actionOnReturn,
                          const std::shared_ptr<AsyncWebSocket>& socket,
                          const oatpp::String& message) = 0;
    
    /**
     * Called when "pong" frame received
     */
    virtual Action onPong(oatpp::async::AbstractCoroutine* parentCoroutine,
                          const Action& actionOnReturn,
                          const std::shared_ptr<AsyncWebSocket>& socket,
                          const oatpp::String& message) = 0;
    
    /**
     * Called when "close" frame received
     */
    virtual Action onClose(oatpp::async::AbstractCoroutine* parentCoroutine,
                           const Action& actionOnReturn,
                           const std::shared_ptr<AsyncWebSocket>& socket,
                           v_word16 code, const oatpp::String& message) = 0;
    
    /**
     * Called when "text" or "binary" frame received.
     * When all data of message is read, readMessage is called again with size == 0 to
     * indicate end of the message
     */
    virtual Action readMessage(oatpp::async::AbstractCoroutine* parentCoroutine,
                               const Action& actionOnReturn,
                               const std::shared_ptr<AsyncWebSocket>& socket,
                               p_char8 data, data::v_io_size size) = 0;
    
  };
  
private:
  
  bool checkForContinuation(const Frame::Header& frameHeader);
  
  Action readFrameHeaderAsync(oatpp::async::AbstractCoroutine* parentCoroutine,
                              const Action& actionOnReturn,
                              const std::shared_ptr<Frame::Header>& frameHeader);
  
  /**
   * if(shortMessageStream == nullptr) - read call readMessage() method of listener
   * if(shortMessageStream) - read message to shortMessageStream. Don't call listener
   */
  Action readPayloadAsync(oatpp::async::AbstractCoroutine* parentCoroutine,
                          const Action& actionOnReturn,
                          const std::shared_ptr<Frame::Header>& frameHeader,
                          const std::shared_ptr<oatpp::data::stream::ChunkedBuffer>& shortMessageStream);
  
  Action handleFrameAsync(oatpp::async::AbstractCoroutine* parentCoroutine,
                          const Action& actionOnReturn,
                          const std::shared_ptr<Frame::Header>& frameHeader);
  
private:
  std::shared_ptr<oatpp::data::stream::IOStream> m_connection;
  bool m_maskOutgoingMessages;
  std::shared_ptr<Listener> m_listener;
  v_int32 m_lastOpcode;
  mutable bool m_listening;
public:
  
  /**
   * maskOutgoingMessages for servers should be false. For clients should be true
   */
  AsyncWebSocket(const std::shared_ptr<oatpp::data::stream::IOStream>& connection, bool maskOutgoingMessages)
    : m_connection(connection)
    , m_maskOutgoingMessages(maskOutgoingMessages)
    , m_listener(nullptr)
    , m_lastOpcode(-1)
    , m_listening(false)
  {}
  
  AsyncWebSocket(const AsyncWebSocket&) = delete;
  AsyncWebSocket& operator=(const AsyncWebSocket&) = delete;
  
public:
  
  static std::shared_ptr<AsyncWebSocket> createShared(const std::shared_ptr<oatpp::data::stream::IOStream>& connection, bool maskOutgoingMessages) {
    return std::make_shared<AsyncWebSocket>(connection, maskOutgoingMessages);
  }
  
  std::shared_ptr<oatpp::data::stream::IOStream> getConnection() const {
    return m_connection;
  }
  
  void setListener(const std::shared_ptr<Listener>& listener) {
    m_listener = listener;
  }
  
  Action listenAsync(oatpp::async::AbstractCoroutine* parentCoroutine, const Action& actionOnReturn);
  
  Action writeFrameHeaderAsync(oatpp::async::AbstractCoroutine* parentCoroutine,
                               const Action& actionOnReturn,
                               const std::shared_ptr<Frame::Header>& frameHeader);
  
  Action sendFrameHeaderAsync(oatpp::async::AbstractCoroutine* parentCoroutine,
                              const Action& actionOnReturn,
                              const std::shared_ptr<Frame::Header>& frameHeader,
                              bool fin, v_word8 opcode, v_int64 messageSize);
  
  Action sendOneFrameAsync(oatpp::async::AbstractCoroutine* parentCoroutine,
                           const Action& actionOnReturn,
                           bool fin, v_word8 opcode, const oatpp::String& message);
  
  Action sendCloseAsync(oatpp::async::AbstractCoroutine* parentCoroutine, const Action& actionOnReturn, v_word16 code, const oatpp::String& message);
  
  Action sendCloseAsync(oatpp::async::AbstractCoroutine* parentCoroutine, const Action& actionOnReturn);
  
  Action sendPingAsync(oatpp::async::AbstractCoroutine* parentCoroutine, const Action& actionOnReturn, const oatpp::String& message);
  
  Action sendPongAsync(oatpp::async::AbstractCoroutine* parentCoroutine, const Action& actionOnReturn, const oatpp::String& message);
  
  Action sendOneFrameTextAsync(oatpp::async::AbstractCoroutine* parentCoroutine, const Action& actionOnReturn, const oatpp::String& message);
  
  Action sendOneFrameBinaryAsync(oatpp::async::AbstractCoroutine* parentCoroutine, const Action& actionOnReturn, const oatpp::String& message);
  
};
  
}}

#endif /* oatpp_websocket_AsyncWebSocket_hpp */
