// Header: IPv4Address.h
//////////////////////////////////////////////////////////////////////////
#ifndef _IPV4Address_H_H_ 
#define _IPV4Address_H_H_ 

#include <string>
#include "BaseTypes.h"

//////////////////////////////////////////////////////////////////////////
// Class: CIPv4Address
// Represents an IPv4 IP address and port.
//////////////////////////////////////////////////////////////////////////
class CIPv4Address
{
	public:
		
		// Constructor: CIPv4Address
		// Default constructor.		
		CIPv4Address();
		
		// Constructor: CIPv4Address
		// Copy constructor.		
		CIPv4Address(const CIPv4Address &a);
		
		// Constructor: CIPv4Address
		// Creates an instance with IP address and port number from std::string
		// Example strings: "192.168.0.1", "192.168.0.1:6000"
		CIPv4Address(const std::string &s);
		
		// Constructor: CIPv4Address
		// Creates an instance with IP address and port number from C-style string
		// Example strings: "192.168.0.1", "192.168.0.1:6000"
		CIPv4Address(const char *a);
		
		// Destructor: CIPv4Address
		// Default destructor.
		virtual ~CIPv4Address();

		// Operator: ==
		// Compare two <CIPv4Address> objects
		//
		// Parameters:
		// a - const reference to <CIPv4Address> object
		//
		// Returns:
		// True if objects data is equal and False otherwise
		bool operator==(const CIPv4Address &a) const;
		
		// Operator: =
		// Initialize <CIPv4Address> object with address and ports parsed 
		// from std::string
		//
		// Parameters:
		// str - const reference to string object
		//
		// Returns:
		// Reference to himself
		CIPv4Address& operator=(const std::string &str);
		
		// Operator: =
		// Copy one object data to another
		//
		// Parameters:
		// a - const reference to source <CIPv4Address> object
		//
		// Returns:
		// Reference to himself
		CIPv4Address& operator=(const CIPv4Address &a);
		
		// Method: Assign
		// Copy one object data to another
		//
		// Parameters:
		// a - const reference to source <CIPv4Address> object		
		CIPv4Address& Assign(const CIPv4Address &a);
		
		// Method: Invalidate
		// Invalidate object, assign to address and port invalid values. 
		// bad_ip = 0xFFFFFFFF, bad_port = 0xFFFF.
		void Invalidate();

		// Method: AsString
		// Return string representation of <CIPv4Address> object.
		//
		// Returns:
		// String that contain "Address:Port" or "Address".		
		std::string AsString() const;

		// Method: IP
		// Initialize <CIPv4Address> object with address and ports parsed 
		// from C-styled string.
		//
		// Parameters:
		// a - String that contain "Address:Port" or "Address".
		void	IP(const char *a);

		// Method: Port
		// Initialize <CIPv4Address> object with port parsed from C-styled string.
		//
		// Parameters:
		// a - String that contain "Port".		
		void	Port(const char *port);
		
		// Method: IP
		// Initialize <CIPv4Address> object with address from unsigned integer 
		// that contain ip address in network bytes order.
		//
		// Parameters:
		// a - uint32 IP address.
		void	IP(uint32 a);

		// Method: Port
		// Initialize <CIPv4Address> object with address from unsigned short 
		// integer that contain port number in network bytes order.
		//
		// Parameters:
		// a - uint16 port.
		void	Port(uint16 port);	//network order
		
		// Method: IP
		// Return ip address in network bytes order
		//
		// Returns:
		// IP address in network bytes order.		
		uint32	IP() const;		//network order
		
		// Method: IP
		// Return port in network bytes order
		//
		// Returns:
		// Port in network bytes order.
		uint16	Port() const;	//network order

		// Method: IsIP
		// Return true if object contain valid IP address
		//
		// Returns:
		// True if ip is valid otherwise false.		
		bool	IsIP() const;
		
		// Method: IsIP
		// Return true if object contain valid port
		//
		// Returns:
		// True if port is valid otherwise false.
		bool	IsPort() const;

	protected:
		uint16	m_uPort;
		uint32	m_uIP;
};

#endif//_IPV4Address_H_H_ 
