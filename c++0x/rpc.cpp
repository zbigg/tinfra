//#include <tinfra/mo.h>

#include <iostream>
#include <typeinfo>
#include <vector>
#include <string>
#include <functional>

template <typename PROTO>
struct remote_function_serializer {

	template <typename FirstArgument, typename ...Arguments>
	void serialize_to(typename  PROTO::RequestBuilder& rb, int n, FirstArgument&& arg, Arguments&&... rest)
	{
		serialize_to(rb, n, std::forward<FirstArgument>(arg));
		serialize_to(rb, n+1, std::forward<Arguments>(rest)...);
	}
	/*
	template <typename OneArgument>
	void process(int n, OneArgument const& arg)
	{
		std::cout << "param in " << n << "(" << typeid(OneArgument).name() << ") = \"" << arg << "\"\n";
	}
	*/

	/*	
	template <typename OneArgument>
	void process(int n, OneArgument arg)
	{
		std::cout << "param in " << n << "(" << typeid(OneArgument).name() << ") = \"" << arg << "\"\n";
	}
	*/

	template <typename Argument>
	void serialize_to(typename PROTO::RequestBuilder& request_builder, int n, Argument&& arg)
	{
		request_builder.addArgument<Argument>(arg);
		std::cout << "param inout " << n << "(" << typeid(Argument).name() << ") = \"" << arg << "\"\n";
	}

	template <typename Argument>
	void serialize_from(typename PROTO::response_type const& response, Argument& arg)
	{
		std::cout << "deciphering response\n";
	}
};

struct remote_call_params {
	int max_retries;
	int initial_timeout;
	int retry_timeout;
};

template <typename PROTO, typename RET, typename... Arguments>
class remote_function;

template <typename PROTO>
class remote_function_base {
	typedef typename PROTO::request_type  request_type;
	typedef typename PROTO::response_type response_type;
protected:
	remote_function_base(typename PROTO::connection_type const& connection_):
		connection(connection_)
	{
	}

	remote_function_base(typename PROTO::address const& address_):
		connection(address_)
	{
	}

	void send_request(request_type const& req, response_type& resp)
	{
		resp  = this->connection.send_request( req);

		/*
		remote_failure_info fi;
		if( PROTO::get_failure_info(remote_response, fi) ) {
			if( fi.is_logic_error() ) {
				throw std::logic_error(fi.message);
			} else {
				throw std::runtime_error(fi.message);
			}
		}
		*/
		// and now for something completly different
		// such an communication can throw multitude of errrors
		// - fatal transport error -> well examples?
		// - transient transport error -> reconnect possible, and retry policy 
		//   should be somewhere. where?
		//    transport?
		//      transport knows better how to handle specific error situations
		//      -> it should handle timeouts
		//      -> it may throw transient error
		//      -> it may throw transport error
		//    protocol?
		//      duplication!?
		//      it should had
		//      -> it shouldn't cast transient errors
		//      -> after recovery attemts are done and it still fails, transient error
		//         is converted to transport error (cause may be deriver from original transient error
		//         + info about failed attempts to recover)
		///     -> it may throw protocol error
		//    here?
		//      one common rule for at least managing retry count, 
		//        - timeout still has to be handled in protocol on transport 
		//          level
		//      -> it should handle retries
		// - application
		//      -> will handle transport_error
		//      -> will handle protocol_error
		//      -> will handle application error? ... how!?
	}

	template <typename RET>
	RET call(typename PROTO::request_type remote_request)
	{
		response_type remote_response;
		this->send_request( remote_request, remote_response );
		
		// deserialize response
		typename PROTO::serializer serializer;
		RET response;
		serializer.serialize_from(remote_response, response);
		return response;
	}
private:
	typename PROTO::connection_type connection;
};
template <typename PROTO, typename RET, typename... Arguments>
class remote_function<PROTO, RET (Arguments...)> 
	: private remote_function_base<PROTO>
{
	
public:
	remote_function(typename PROTO::connection const& connection_)
		: remote_function_base<PROTO>(connection_)
	{
	}
	remote_function(typename PROTO::address const& address)
		: remote_function_base<PROTO>(address)
	{
	}

	RET operator()(Arguments... args)
	{
		typename PROTO::serializer serializer;
		// serialize and make request
		typename PROTO::RequestBuilder request_builder;
		serializer.serialize_to(request_builder, 0, std::forward<Arguments>(args)...);

		PROTO::Request request = request_builder.build();
		return this-> template call<RET>(request);
	}
};

struct XMLRPCProtocol {
	typedef std::string address;

	struct param {
		std::string name;
		std::string type;
		std::string value;
	};
	typedef remote_function_serializer<XMLRPCProtocol> serializer;
	typedef std::vector< param > request_type;

	typedef param response_type;

	struct connection {
		connection(std::string const& address);
		response_type send_request(request_type const& request);
	};

	typedef connection connection_type;
};


XMLRPCProtocol::connection::connection(const std::string& address)
{
}

XMLRPCProtocol::response_type XMLRPCProtocol::connection::send_request(XMLRPCProtocol::request_type const& request)
{
	XMLRPCProtocol::response_type result;
	return result;
}

int main()
{
	remote_function<XMLRPCProtocol, int(int,std::string,int)> xxx("http://localhost/xmlrpc");


	std::function<int(int, std::string, int)> fff = xxx;

	std::string result("zbyszek");
	fff(1,result,3);
}


