#!/usr/local/bin/perl 
#
# rfc931.pl
# Usage
# require "rfc931.pl";
# &rfc931_name(there[,here]);
# where there is a sockaddr.
# 
# for example
# &rfc931_name(getpeername(STDIN),getsockname(STDIN));
# &rfc931_name(getpeername(SOCK),getsockname(SOCK));
#
# require "sys/socket.ph";
package rfc931;

sub timeout {
	($sig) = @_;
	$time_out =1;
	goto longjmp;
}

$RFC931_TIMEOUT=20;
$DEFAULT_RFC931_PORT=113;

sub main'rfc931_name {
        $sockaddr = 'S n a4 x8';
	($there,$here)=@_;
#	print "here \r\n";
	$here=getsockname(STDOUT) if (-S STDOUT && !$here) ;
	($family,$thisport,$thisaddr)=unpack($sockaddr,$here);
	($localaddr)=unpack('C4',$thisaddr);
#	print "$localaddr $remoteaddr\n";
#	return "" if !there;
	$there=getpeername(STDOUT) if  (-S STDOUT && !$there) ;
#	return "" if !$here;
	($family,$thatport,$thataddr)=unpack($sockaddr,$there);
	($remoteaddr)=unpack('C4',$thataddr);
#	print "$localaddr $remoteaddr\n";
	($name,$alias,$proto)=getprotobyname('tcp');
	($name,$alias,$RFC931_PORT)=getservbyname('ident','tcp');
	$RFC931_PORT = $DEFAULT_RFC931_PORT unless $RFC931_PORT;
#	socket(IDENT,&main'PF_INET,&main'SOCK_STREAM,$proto) || return "";
	socket(IDENT,2,1,$proto) || return "";
#	$this=pack($sockaddr,&main'AF_INET,0,$thisaddr);
	$this=pack($sockaddr,2,0,$thisaddr);
	bind(IDENT,$this) || return ""; 
	$SIG{'ALRM'} = 'timeout';
	$time_out = 0;
	longjump: if ($time_timout){
		close(IDENT);
		return $result;
	}
	alarm($RFC931_TIMEOUT);
# connect to the RFC931 daeomon
	$that=pack($sockaddr, 2 ,$RFC931_PORT,$thataddr);
	if (!connect(IDENT,$that)) {
		close(IDENT);
		alarm(0);
		return "";
	}
	select(IDENT); $| = 1; select(STDOUT);
	printf IDENT "%u,%u\r\n", $thatport,$thisport;
#	printf "%u,%u\r\n", $thatport,$thisport;
	$_=<IDENT>;
#	print $_;
#	sscanf($_,'%u , %u : USERID :%*[^:]:%255s',$remote,$local,$user);
	($remote,$local,$user) = /^(\d+) , (\d+) : USERID : \w\w\w\w : (\w+)/;
#	print $user;
	alarm(0);
	close(IDENT);
	return $user;
}
# &main'rfc931_name;
1;
