//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#ifndef __theg_handler_h__
#define __theg_handler_h__

#include <istream>
#include <ostream>
#include <map>
#include <string>

template <typename WF>
class BaseMessageHandler {
public:

    virtual ~BaseMessageHandler() {}
    virtual void onMessage(typename WF::RequestType& raw_request, typename WF::ResponseType& raw_reply) = 0;
};

template <typename WF, typename MessageReceiver>
class CallbackMessageHandler: public BaseMessageHandler<WF> {
public:
    typedef void (MessageReceiver::* Callback)(typename WF::RequestType&, typename WF::ResponseType&);

    CallbackMessageHandler(MessageReceiver* instance, Callback callback):
        instance_(instance), callback_(callback)
    {}
        
    virtual void onMessage(typename WF::RequestType& request, typename WF::ResponseType& reply)
    {
        (instance_->*callback_)(request, reply);
    }
private:
    MessageReceiver* instance_;
    Callback callback_;
};

template <typename WF, typename Request, typename Reply, typename MessageReceiver>
class AutoConverterMessageHandler: public BaseMessageHandler<WF> {
public:
    typedef void (MessageReceiver::* Callback)(Request const&, Reply&);

    AutoConverterMessageHandler(MessageReceiver* instance, Callback callback):
        instance_(instance), callback_(callback)
    {}
    virtual void onMessage(typename WF::RequestType& raw_request, typename WF::ResponseType& raw_reply)
    {
        Request custom_request;
        Converter<WF>::convert(raw_request, custom_request);
        Reply custom_reply;
        (instance_->*callback_)(custom_request, custom_reply);
        Converter<WF>::convert( custom_reply, raw_reply );
    }
private:
    MessageReceiver* instance_;
    Callback callback_;
};

template <typename WF>
class MessageReceiver {
public:
    virtual ~MessageReceiver() {}
        
    virtual void onMessage(std::string const& name, typename WF::RequestType& request, typename WF::ResponseType& reply) = 0;
};

template <typename WF>
class GenericMessageReceiver: public MessageReceiver<WF> {
public:    
    void onMessage(std::string const& name, typename WF::RequestType& request, typename WF::ResponseType& reply)
    {
        typename handler_list_t::const_iterator ih = handlers_.find(name);
        if( ih == handlers_.end() ) {
            std::cout << "bad message: " << name << std::endl;
            return;
        }
        
        ih->second->onMessage(request, reply);
    }
    
    template <typename Request, typename Reply,typename F>
    void registerSerHandler(std::string const& name, F* instance, void (F::* callback)(Request const&, Reply&) )
    {
        handlers_.insert( std::make_pair(name, new SerializingMessageHandler<WF, Request, Reply,F>(instance, callback)) );
    }
    template <typename F>
    void registerRawHandler(std::string const& name, F* instance, void (F::* callback)(typename WF::RequestType&, typename WF::ResponseType&))
    {
        handlers_.insert( std::make_pair(name, new CallbackMessageHandler<WF,F>(instance, callback)) );
    }
private:
    typedef std::map<std::string, BaseMessageHandler<WF>*> handler_list_t;
    handler_list_t handlers_;
};

#endif
