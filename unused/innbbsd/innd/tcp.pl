#!/usr/local/bin/perl

package main;
sub tcp'getostype {
   chop($_=`uname -a`); 
   if ( /^SunOS/i ) {
       ($os,$host,$ver)=split(/\s+/,$_);
        if ( $ver =~ /5\./ ) {
                return "Solaris";            
        } else {
                return "BSD";
        }
   } elsif (/^HP-UX/i) {
        return "SYSV";
   } elsif (/^AIX/i ) {
        return "AIX";
   } elsif (/^OSF1/i) {
        return "SYSV";
   }
}
$tcp'OS = &tcp'getostype();
if ( $OS eq "Solaris") {
      eval 'sub SOCK_STREAM {2;}';
      eval 'sub SOCK_DGRAM {1;}';
    } else {
      eval 'sub SOCK_STREAM {1;}';
      eval 'sub SOCK_DGRAM {2;}';
    }
    eval 'sub SOCK_RAW {3;}';
    eval 'sub SOCK_RDM {4;}';
    eval 'sub SOCK_SEQPACKET {5;}';
    eval 'sub SO_DEBUG {0x0001;}';
    eval 'sub SO_ACCEPTCONN {0x0002;}';
    eval 'sub SO_REUSEADDR {0x0004;}';
    eval 'sub SO_KEEPALIVE {0x0008;}';
    eval 'sub SO_DONTROUTE {0x0010;}';
    eval 'sub SO_BROADCAST {0x0020;}';
    eval 'sub SO_USELOOPBACK {0x0040;}';
    eval 'sub SO_LINGER {0x0080;}';
    eval 'sub SO_OOBINLINE {0x0100;}';
    eval 'sub SO_DONTLINGER {(~ &SO_LINGER);}';
    eval 'sub SO_SNDBUF {0x1001;}';
    eval 'sub SO_RCVBUF {0x1002;}';
    eval 'sub SO_SNDLOWAT {0x1003;}';
    eval 'sub SO_RCVLOWAT {0x1004;}';
    eval 'sub SO_SNDTIMEO {0x1005;}';
    eval 'sub SO_RCVTIMEO {0x1006;}';
    eval 'sub SO_ERROR {0x1007;}';
    eval 'sub SO_TYPE {0x1008;}';
    eval 'sub SOL_SOCKET {0xffff;}';
    eval 'sub AF_UNSPEC {0;}';
    eval 'sub AF_UNIX {1;}';
    eval 'sub AF_INET {2;}';
    eval 'sub AF_IMPLINK {3;}';
    eval 'sub AF_PUP {4;}';
    eval 'sub AF_CHAOS {5;}';
    eval 'sub AF_NS {6;}';
    eval 'sub AF_NBS {7;}';
    eval 'sub AF_ECMA {8;}';
    eval 'sub AF_DATAKIT {9;}';
    eval 'sub AF_CCITT {10;}';
    eval 'sub AF_SNA {11;}';
    eval 'sub AF_DECnet {12;}';
    eval 'sub AF_DLI {13;}';
    eval 'sub AF_LAT {14;}';
    eval 'sub AF_HYLINK {15;}';
    eval 'sub AF_APPLETALK {16;}';
    eval 'sub AF_NIT {17;}';
    eval 'sub AF_802 {18;}';
    eval 'sub AF_OSI {19;}';
    eval 'sub AF_X25 {20;}';
    eval 'sub AF_OSINET {21;}';
    eval 'sub AF_GOSIP {22;}';
    eval 'sub AF_MAX {21;}';
    eval 'sub PF_UNSPEC { &AF_UNSPEC;}';
    eval 'sub PF_UNIX { &AF_UNIX;}';
    eval 'sub PF_INET { &AF_INET;}';
    eval 'sub PF_IMPLINK { &AF_IMPLINK;}';
    eval 'sub PF_PUP { &AF_PUP;}';
    eval 'sub PF_CHAOS { &AF_CHAOS;}';
    eval 'sub PF_NS { &AF_NS;}';
    eval 'sub PF_NBS { &AF_NBS;}';
    eval 'sub PF_ECMA { &AF_ECMA;}';
    eval 'sub PF_DATAKIT { &AF_DATAKIT;}';
    eval 'sub PF_CCITT { &AF_CCITT;}';
    eval 'sub PF_SNA { &AF_SNA;}';
    eval 'sub PF_DECnet { &AF_DECnet;}';
    eval 'sub PF_DLI { &AF_DLI;}';
    eval 'sub PF_LAT { &AF_LAT;}';
    eval 'sub PF_HYLINK { &AF_HYLINK;}';
    eval 'sub PF_APPLETALK { &AF_APPLETALK;}';
    eval 'sub PF_NIT { &AF_NIT;}';
    eval 'sub PF_802 { &AF_802;}';
    eval 'sub PF_OSI { &AF_OSI;}';
    eval 'sub PF_X25 { &AF_X25;}';
    eval 'sub PF_OSINET { &AF_OSINET;}';
    eval 'sub PF_GOSIP { &AF_GOSIP;}';
    eval 'sub PF_MAX { &AF_MAX;}';
    eval 'sub SOMAXCONN {5;}';
    eval 'sub MSG_OOB {0x1;}';
    eval 'sub MSG_PEEK {0x2;}';
    eval 'sub MSG_DONTROUTE {0x4;}';
    eval 'sub MSG_MAXIOVLEN {16;}';
    eval 'sub MSG_MAXIOVLEN {16;}';
    eval 'sub WNOHANG {1;}';
    eval 'sub WUNTRACED {2;}';

#package tcp;

$defaultport = 'nntp';
$defaultserver = 'news.csie.ncu.edu.tw.';
$ENV{'PATH'}='/bin:/usr/ucb:/usr/etc';

# The Internet TCP client algorithm 
# 1. Find the IP address and protocol port number of the server
#    with which communication is desired. (gethostbyname,getservbyname)
# 2. Allocate a socket.  (socket)
# 3. Specify that the connection needs an arbitary, unsed protocol
#    port on the local machine, and allow TCP to choose one. (bind)
# 4. Connect the socket to the server. (connect)
# 5. Communicate with the server using the application-level protocol
#    (this usually involves sending requests and awaiting replies)
# 6. close the connection.
#    
# reference: 
# socket addr, internet style structure for Sun-OS
# include <netinet/in.h>
# struct sockaddr_in {
#   short   sin_family;
#   u_short sin_port;
#   struct  in_addr sin_addr;
#   char    sin_zero[8];
# }
# ( 'S n a4 x8' template for perl pack)
# Usage
# &tcpinetclient(FILEHANDLE[,hostname,portno]);
# for example,
# &tcpinetclient(NNTP,'news.csie.ncu.edu.tw','nntp');
# print NNTP "help\r\n";
# $_ = <NNTP>;
# print;

sub main'tcpinetclient {
  local(*S,$server,$port)=@_;
  $port = $defaultport unless $port; 
  $server = $defaultserver unless $server; 
  local($hostname);
  chop($hostname = `hostname`);
  local($sockaddr)= 'S n a4 x8';
  local($name,$aliases,$proto)=getprotobyname('tcp');
  local($name,$aliases,$port)=getservbyname($port,'tcp')
	unless $port =~ /^\d+$/;
#  print "port number in tcpinetclient $port\n";
  local($name, $aliases, $type, $len, $thisaddr) = gethostbyname($hostname);
  local($name, $aliases, $type, $len, $thataddr) = gethostbyname($server);
  socket(S, &main'PF_INET, &main'SOCK_STREAM, $proto) || die "socket: $!";
  local($this) = pack($sockaddr, &main'AF_INET, 0, $thisaddr);
# accept connect from any port (0)   
  local($that) = pack($sockaddr, &main'AF_INET, $port, $thataddr);
# bind(S, $this) || die "bind to $hostname: $!";
  connect(S, $that) || die "connect to $port: $!";
  select(S); $| = 1; select(STDOUT);
  1;
}


# reference: socket for unix domain in Sun-OS
# include <socket.h>
# struct sockaddr {
#   u_short sa_family;
#   char    sa_data[14];
# }
# ('S a14' perl template for perl) 
# usage
# &tcpunixclient(FILEHANDLE,path);
# for example,
# &tcpunixclient(LOCAL,"/tmp/unixsock$$");

#$defaultpath="/tmp/unixsock$$";
$defaultpath='/tmp/sample';
# only 14 chars can be used
sub main'tcpunixclient {
  local(*S,$path)=@_;
  $path = $defaultpath unless $path; 
  local($sockaddr)= 'S a14';
  socket(S, &PF_UNIX, &SOCK_STREAM, 0) || die "socket: $!";
  $that = pack($sockaddr, &AF_INET, $path);
  connect(S, $that) || die "connect to $path: $!";
  select(S); $| = 1; select(STDOUT);
  1;
}


# o Interactive, Connection-Orientd Server
# o Interactive, Connectionless Server
# o Concurrent, Connectionless Server 
#   server repeatedly call "recvform" and let slave use "sendto"
#   to reply the client.
# o concurrent, connection-oriented server algorithm
# Master 1. Create a socket and bind to the well-known address
#	    for the service being offered. Leave the socket unconnected
#	    (socket,bind)
# Master 2. Place the socket in passive mode, makeing it ready for used
#	    by a server. (listen)
# Master 3. Repeatedly call accept to receive the next request from
#	    a client, and create a new slave process to handle the  
#	    response.  (accept)
# Slave  1. Receive a connection request (i.e., socket for the connection)
#	    upon creation.
# Slave  2. Interact with the client using the connection: read request(s)
#	    and send back response(s).
# Slave  3. Close the connection and exit. The slave process exits
#	    after handling all requests from one client.
#
# Usage
# &tcpinetserver([port-no,service-routine,before,each]);
# for example
# &tcpinetserver(1234,'simple_service');
#

$defaultserverport=1234;
$defaultserviceroutine="simple_service";

sub simple_service {
	local(*S)=@_;
	while (<S>) {
		if (/quit/) {
			return(0);
		} elsif (/help/) {
			print S <<"EOF";
This is a simple sample server \r
available command \r
help quit \r
EOF
		} else {
			print S "Unknown command\r\n";
		}
	}
}

sub reapchild {
	while (waitpid(-1,&WNOHANG|&WUNTRACED)>0) {
#	print "reapchild\n";
#	while (waitpid(-1,&WNOHANG)>0) {
#		print "reaping child\n";
		next;
	}
1;
}
#sub reapchild {
#	while (1) {
#		$pid = waitpid(-1,&WNOHANG);
#		last if ($pid < 1);
#	}
#}

sub dokill {
	kill 9,0;
}

sub main'tcpinetserver {
  local($port,$service,$before,$each)=@_;
  if ($port != 0) {
    $port = $defaultserverport unless $port; 
  }
  $service = $defaultserviceroutine unless $service; 
  local($sockaddr)= 'S n a4 x8';
  local($name,$aliases,$proto)=getprotobyname('tcp');
  local($name,$aliases,$port)=getservbyname($port,'tcp')
	unless $port =~ /^\d+$/;
  local(*S,*NS);
  chdir("/");
  socket(S, &PF_INET, &SOCK_STREAM, $proto) || die "socket: $!";
  setsockopt(S,main'SOL_SOCKET,main'SO_REUSEADDR,1);
  setsockopt(S,main'SOL_SOCKET,main'SO_LINGER,0);
  if ($port == 0) {
    local($hostname); 
    chop($hostname = `hostname`);
    local($name, $aliases, $type, $len, $thisaddr) = gethostbyname($hostname);
  } else {
	$thisaddr = "\0\0\0\0";
  }
  $this = pack($sockaddr, &AF_INET, $port, $thisaddr);
  # can accept connection from $port, to any port in client
  bind(S, $this) || die "bind: $!";
  select(S); $| = 1; select(STDOUT);
  $SIG{'CHLD'} = 'reapchild';
  $SIG{'HUP'} = 'IGNORE';
  $SIG{'INT'} = 'dokill';
  $SIG{'TERM'} = 'dokill';

  do $before(S) if ($before);
  listen(S, 5) || die "connect: $!";
  for (;;) {
#    print "Listening again\n";
    ($addr = accept(NS,S)) || next;  
# || die "accept: $!\n";
    select(NS); $| = 1; select(STDOUT);
#    print "accept ok\n";
    ($af,$port,$inetaddr) = unpack($sockaddr,$addr);
    @inetaddr = unpack('C4',$inetaddr);
#    print "$af $port @inetaddr\n";
    FORK: {
	last if ( $pid = fork) ;
 	if (defined $pid) {
		close(S);
    		$return = do $service(NS);
		close(NS);
		exit($return);
	}
	if ($! =~ /No more process/) {
		sleep 5;
		redo FORK;
	} else {
		die "Can't fork: $!\n";
	}
    } # FORK
    do $each(NS) if ($each);
    close(NS);
  } # listen forever and fork a client to handle service request
} # end tcpinetserver

# single proecess, connection-oriented server for internet
sub main'tcpinetsingleserver {
  local($port,$service,$beforeservice,$each)=@_;
  if ( $port != 0) {
    $port = $defaultserverport unless $port; 
  }
  $service = $defaultserviceroutine unless $service; 
  local($sockaddr)= 'S n a4 x8';
  local($name,$aliases,$proto)=getprotobyname('tcp');
  local($name,$aliases,$port)=getservbyname($port,'tcp')
	unless $port =~ /^\d+$/;
  local(*S,*NS);
  chdir("/");
  socket(S, &PF_INET, &SOCK_STREAM, $proto) || die "socket: $!";
  if ($port == 0) {
    local($hostname); 
    chop($hostname = `hostname`);
    local($name, $aliases, $type, $len, $thisaddr) = gethostbyname($hostname);
  } else {
	$thisaddr = "\0\0\0\0";
  }
  $this = pack($sockaddr, &AF_INET, $port, $thisaddr);
  # can accept connection from $port, to any port in client
  bind(S, $this) || die "bind: $!";
  select(S); $| = 1; select(STDOUT);
  $SIG{'CHLD'} = 'reapchild';
  $SIG{'HUP'} = 'IGNORE';
  $SIG{'INT'} = 'dokill';
  do $beforeservice(S) if ($beforeservice);
  listen(S, 5) || die "connect: $!";
  for (;;) {
#    print "Listening again in single server\n";
    ($addr = accept(NS,S)) || next;  
    select(NS); $| = 1; select(STDOUT);
#    print "accept ok\n";
    ($af,$port,$inetaddr) = unpack($sockaddr,$addr);
    @inetaddr = unpack('C4',$inetaddr);
#    print "$af $port @inetaddr\n";
    $return = do $service(NS);
    do $each(NS) if ($each);
    close(NS);
  }
}

# Concurrent, Connection-oriented server for UNIX domain
$path=$defaultpath;
sub doremove {
	unlink $path;
	kill 9,0;
}

sub simple_unixservice {
	local(*S)=@_;
	while (<S>) {
		if (/quit/) {
			return(0);
		} elsif (/help/) {
			print S <<"EOF";
This is a simple sample server \r
available command \r
help quit \r
EOF
		} else {
			print S "Unknown command\r\n";
		}
	}
}

sub main'tcpunixserver {
  ($path,$service)=@_;
  $path = $defaultpath unless $path; 
  $service = 'simple_unixservice' unless $service; 
  local($sockaddr)= 'S a14';
  socket(S, &PF_UNIX, &SOCK_STREAM, 0) || die "socket: $!";
  $this = pack($sockaddr, &AF_INET, $path);
  bind(S, $this) || die "bind: $!";
  select(S); $| = 1; select(STDOUT);
  $SIG{'CHLD'} = 'reapchild';
  $SIG{'HUP'} = 'IGNORE';
  $SIG{'INT'} = 'doremove';
  $SIG{'TERM'} = 'doremove';

  for (;;) {
    listen(S, 5) || die "connect: $!";
#    print "Listening again\n";
    ($addr = accept(NS,S)) || next;
    select(NS); $| = 1; select(STDOUT);
#    print "accept ok\n";
    FORK: {
	last if ( $pid = fork) ;
 	if (defined $pid) {
		close(S);
    		$return = do $service(NS);
		close(NS);
		exit($return);
	}
	if ($! =~ /No more process/) {
		sleep 5;
		redo FORK;
	} else {
		die "Can't fork: $!\n";
	}
    } # FORK
    close(NS);
 }
}

sub main'simpleunixclient {
	local($path)= @_;
	local(*S,$rin,$rout);
	($path)= $defaultpath unless $path;
	&tcpunixclient(S,$path) || die "can't connect: $!\n";
	$rin='';
	vec($rin,fileno(STDIN),1)=1;
	vec($rin,fileno(S),1)=1;
	for (;;) {
		(($nf=select($rout=$rin,undef,undef,undef))>=0) || die "select: $!\n";
		if (vec($rout,fileno(S),1)) {
			$i=read(S,$n,1);
			if ($i) {
				print $n;
			} else {
				print "bye\n";
				last;
			}
		}
		if (vec($rout,fileno(STDIN),1)) {
			$_ = <STDIN>;
			chop;
			print S $_,"\r\n";
		}
	}
}

sub main'remotehostname {
    (*WNRP) = @_;
    local($there,$here)=(getpeername(WNRP),getsockname(WNRP));
    local($sockaddr)= 'S n a4 x8';
    local($family,$thisport,$thisaddr)=unpack($sockaddr,$here);
    local($family,$thatport,$thataddr)=unpack($sockaddr,$there);
    local(@localaddr)=unpack('C4',$thisaddr);
    local(@remoteaddr)=unpack('C4',$thataddr);
    local($hostname)=gethostbyaddr($thisaddr,&AF_INET);
    local($remotehostname)=gethostbyaddr($thataddr,&AF_INET);
    return ($remotehostname);
}
