//            Copyright (c) Glyn Matthews 2010.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)


#ifndef __BOOST_NETWORK_PROTOCOL_XMPP_CLIENT_INC__
# define __BOOST_NETWORK_PROTOCOL_XMPP_CLIENT_INC__


# include <boost/noncopyable.hpp>
# include <boost/network/traits/string.hpp>
# include <boost/cstdint.hpp>
# include <boost/bind.hpp>
# include <boost/network/protocol/xmpp/namespaces.hpp>
# include <boost/network/protocol/xmpp/message.hpp>
# include <boost/network/protocol/xmpp/presence.hpp>
# include <boost/network/protocol/xmpp/iq.hpp>
# include <boost/network/protocol/xmpp/error.hpp>
# include <boost/network/detail/xml_wrappers/element.hpp>
# include <boost/network/detail/xml_wrappers/element_io.hpp>
# include <boost/network/detail/xml_wrappers/traits/parser_backend.hpp>
# include <boost/network/detail/xml_wrappers/parser_backends/expat_parser.hpp>
# include <boost/asio/io_service.hpp>
# include <boost/asio/placeholders.hpp>
# include <boost/asio/ip/tcp.hpp>
# include <boost/asio/write.hpp>
# include <boost/asio/read.hpp>
# include <boost/range/algorithm/fill.hpp>
# include <deque>


# include <iostream>


namespace boost {
namespace network {
namespace xmpp {
template <
    class Tag,
    unsigned version_major,
    unsigned version_minor,
    class Handler
    >
class basic_client : boost::noncopyable {

private:

    typedef basic_client<Tag, version_major, version_minor, Handler> this_type;
    typedef typename detail::parser_backend<Tag>::type parser_type;
    
public:

    typedef typename string<Tag>::type string_type;
    
    typedef basic_message<Tag> message_type;
    typedef basic_presence<Tag> presence_type;
    typedef basic_iq<Tag> iq_type;
    typedef basic_error<Tag> error_type;
    
    explicit basic_client(Handler &handler);

    ~basic_client();

    void set_lang(const string_type &lang);

    void connect(const string_type &jid,
                 const string_type &password);

    void run();

    void disconnect();

    void send_message(const message_type &message);

    void send_presence(const presence_type &presence);

    void send_iq(const iq_type &iq);

    string_type get_jid() const;

private:

    void handle_connect(const boost::system::error_code &error,
                        boost::asio::ip::tcp::resolver::iterator iterator);
    void handle_write_open_stream(const boost::system::error_code &ec);
    void handle_read_open_stream(const boost::system::error_code &ec);
    void handle_read_starttls(const boost::system::error_code &ec);
    void handle_write_starttls(const boost::system::error_code &ec);
    // void handle_read_stanza(const boost::system::error_code &ec);
    void handle_disconnect();

    Handler &handler_;

    // io_service
    boost::asio::io_service io_service_;
    boost::asio::io_service::work work_;

    // tcp socket
    boost::asio::ip::tcp::socket socket_;
    
    // tls
    // sasl

    // parameters, such as jid, domain, port etc.
    string_type jid_;

    // xml parser
    parser_type parser_;

    // std::deque<std::string> stanza_queue_;
    std::string write_buffer_, read_buffer_;
    
};


template <
    class Tag,
    unsigned version_major,
    unsigned version_minor,
    class Handler
    >
basic_client<Tag, version_major, version_minor, Handler>::basic_client(Handler &handler)
    : handler_(handler), socket_(io_service_), work_(io_service_) {
    
}


template <
    class Tag,
    unsigned version_major,
    unsigned version_minor,
    class Handler
    >
basic_client<Tag, version_major, version_minor, Handler>::~basic_client() {
    
}

template <
    class Tag,
    unsigned version_major,
    unsigned version_minor,
    class Handler
    >
void basic_client<Tag, version_major, version_minor, Handler>::set_lang(const string_type &lang) {
    
}


template <
    class Tag,
    unsigned version_major,
    unsigned version_minor,
    class Handler
    >
void basic_client<Tag, version_major, version_minor, Handler>::connect(const string_type &jid,
                                                                       const string_type &password) {
    using boost::asio::ip::tcp;
    
    // get the JID domain
    // default port is 5222
    // open socket
    // socket has a state
    // signal connection handler

    jid_ = jid;

    string_type host = "127.0.0.1";
    string_type port = "5221";

    tcp::resolver resolver(io_service_);
    tcp::resolver::query query(host, port);
    tcp::resolver::iterator iterator = resolver.resolve(query);
    socket_.async_connect(
        *iterator,
        boost::bind(&this_type::handle_connect,
                    this,
                    boost::asio::placeholders::error,
                    iterator));
}

template <
    class Tag,
    unsigned version_major,
    unsigned version_minor,
    class Handler
    >
void basic_client<Tag, version_major, version_minor, Handler>::run() {
    io_service_.run();
}

template <
    class Tag,
    unsigned version_major,
    unsigned version_minor,
    class Handler
    >
void basic_client<Tag, version_major, version_minor, Handler>::disconnect() {
    io_service_.post(
        boost::bind(&this_type::handle_disconnect, this));
}

template <
    class Tag,
    unsigned version_major,
    unsigned version_minor,
    class Handler
    >
void basic_client<Tag, version_major, version_minor, Handler>::send_message(const message_type &message) {
    io_service_.post(
        boost::bind(&this_type::handle_write_stanza, this, boost::ref(message)));
}

template <
    class Tag,
    unsigned version_major,
    unsigned version_minor,
    class Handler
    >
void basic_client<Tag, version_major, version_minor, Handler>::send_presence(const presence_type &presence) {
    io_service_.post(
        boost::bind(&this_type::handle_write_stanza, this, boost::ref(presence)));
}

template <
    class Tag,
    unsigned version_major,
    unsigned version_minor,
    class Handler
    >
void basic_client<Tag, version_major, version_minor, Handler>::send_iq(const iq_type &iq) {
    io_service_.post(
        boost::bind(&this_type::handle_write_stanza, this, boost::ref(iq)));
}

template <
    class Tag,
    unsigned version_major,
    unsigned version_minor,
    class Handler
    >
typename basic_client<Tag, version_major, version_minor, Handler>::string_type
basic_client<Tag, version_major, version_minor, Handler>::get_jid() const {
    return jid_;
}

template <
    class Tag,
    unsigned version_major,
    unsigned version_minor,
    class Handler
    >
void basic_client<Tag, version_major, version_minor, Handler>::handle_connect(
    const boost::system::error_code& ec,
    boost::asio::ip::tcp::resolver::iterator iterator) {
    if (!ec) {
        write_buffer_ =
            "<?xml version=\"1.0\"?>"
            "<stream:stream to=\"lola\" "
            "version=\"1.0\" "
            "xmlns=\"jabber:client\" "
            "xmlns:stream=\"http://etherx.jabber.org/streams\">";
        std::cout << "(C) " << write_buffer_ << std::endl;
        boost::asio::async_write(
            socket_, boost::asio::buffer(write_buffer_),
            boost::bind(&this_type::handle_write_open_stream, this,
                        boost::asio::placeholders::error));
    }
    else if (iterator != boost::asio::ip::tcp::resolver::iterator()) {
        socket_.close();
        boost::asio::ip::tcp::endpoint endpoint = *iterator;
        socket_.async_connect(
            endpoint,
            boost::bind(&this_type::handle_connect,
                        this,
                        boost::asio::placeholders::error,
                        ++iterator));
    }
    else {
        // unable to connect
    }
}

template <
    class Tag,
    unsigned version_major,
    unsigned version_minor,
    class Handler
    >
void basic_client<Tag, version_major, version_minor, Handler>::handle_write_open_stream(const boost::system::error_code &ec) {
    if (!ec) {
        read_buffer_.resize(512);
        boost::fill(read_buffer_, 0);
        socket_.read_some(boost::asio::buffer(read_buffer_));
        std::cout << "(S) " << read_buffer_ << std::endl;

        boost::fill(read_buffer_, 0);
        socket_.read_some(boost::asio::buffer(read_buffer_));
        std::cout << "(S) " << read_buffer_ << std::endl;
        detail::basic_element<Tag> element;
        parser_.feed(read_buffer_, &element);
        std::cout << "(S->) " << element << std::endl;

        write_buffer_ =
            "<starttls xmlns='urn:ietf:params:xml:ns:xmpp-tls'/>";
        boost::asio::async_write(socket_, boost::asio::buffer(write_buffer_),
                                 boost::bind(&this_type::handle_write_starttls, this, boost::asio::placeholders::error));
        std::cout << "(C) " << write_buffer_ << std::endl;


        // boost::asio::async_read(socket_, boost::asio::buffer((char *)read_buffer_.data(), read_buffer_.size()),
        //                         boost::bind(&this_type::handle_read_open_stream, this,
        //                                     boost::asio::placeholders::error));
    }
}

template <
    class Tag,
    unsigned version_major,
    unsigned version_minor,
    class Handler
    >
void basic_client<Tag, version_major, version_minor, Handler>::handle_read_open_stream(const boost::system::error_code &ec) {
    if (!ec) {
        std::cout << "(S) " << read_buffer_ << std::endl;
        read_buffer_.resize(512);
        boost::fill(read_buffer_, 0);
        boost::asio::async_read(socket_, boost::asio::buffer((char *)read_buffer_.data(), read_buffer_.size()),
                                boost::bind(&this_type::handle_read_starttls, this,
                                            boost::asio::placeholders::error));
    }
}

template <
    class Tag,
    unsigned version_major,
    unsigned version_minor,
    class Handler
    >
void basic_client<Tag, version_major, version_minor, Handler>::handle_read_starttls(const boost::system::error_code &ec) {
    if (!ec) {
        std::cout << "(S) " << read_buffer_ << std::endl;
        write_buffer_ =
            "<starttls xmlns='urn:ietf:params:xml:ns:xmpp-tls'/>";
        boost::asio::async_write(socket_, boost::asio::buffer(write_buffer_),
                                 boost::bind(&this_type::handle_write_starttls, this, boost::asio::placeholders::error));
        std::cout << "(C) " << write_buffer_ << std::endl;
    }
}

template <
    class Tag,
    unsigned version_major,
    unsigned version_minor,
    class Handler
    >
void basic_client<Tag, version_major, version_minor, Handler>::handle_write_starttls(const boost::system::error_code &ec) {
    if (!ec) {
        // std::string auth = 
        //     "<auth xmlns='urn:ietf:params:xml:ns:xmpp-sasl' mechanism='PLAIN'/>";
        // std::cout << auth << std::endl;
        // bytes_written = boost::asio::write(socket_, boost::asio::buffer(auth));
        // std::cout << "There were " << bytes_written << " bytes written." << std::endl;

        // boost::fill(s_, 0);
        // bytes_read = socket_.read_some(boost::asio::buffer(s_));
        // std::cout << "There were " << bytes_read << " bytes read." << std::endl;
        // std::cout << "(S)  " << s_ << std::endl;

        // string_type es = "</stream:stream>";
        // std::cout << es << std::endl;
        // bytes_written = boost::asio::write(socket_, boost::asio::buffer(es));
        // std::cout << "There were " << bytes_written << " bytes written." << std::endl;
        // 
        // s_.resize(512);
        // boost::fill(s_, 0);
        // bytes_read = socket_.read_some(boost::asio::buffer(s_));
        // std::cout << "There were " << bytes_read << " bytes read." << std::endl;
        // std::cout << "(S)  " << s_ << std::endl;
        
    
        // std::string es =
        //     "<stream:stream to=\"lola\" "
        //     "version=\"1.0\" "
        //     "xmlns=\"jabber:client\" "
        //     "xmlns:stream=\"http://etherx.jabber.org/streams\">";
        // std::cout << es << std::endl;
        // bytes_written = boost::asio::write(socket_, boost::asio::buffer(es));
        // std::cout << "There were " << bytes_written << " bytes written." << std::endl;
    }
}


template <
    class Tag,
    unsigned version_major,
    unsigned version_minor,
    class Handler
    >
void basic_client<Tag, version_major, version_minor, Handler>::handle_disconnect() {
    // close stream
    string_type s = "</stream:stream>";
    socket_.close();
    // handler_.disconnected();
}


template <
    class Handler
    >
struct client : basic_client<tags::default_, 1, 0, Handler> {

    explicit client(Handler &handler)
        : basic_client<tags::default_, 1, 0, Handler>(handler) {
        
    }

};
} // namespace xmpp
} // namespace network
} // namespace boost


#endif // __BOOST_NETWORK_PROTOCOL_XMPP_CLIENT_INC__
