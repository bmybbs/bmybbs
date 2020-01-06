#!/usr/local/bin/perl

$BBSLIB = 1;

sub initial_bbs
{
    if( ! open( FN, "$inndhome/bbsname.bbs" ) ) {
	return 0;
    }
    while( <FN> ) {
	($mybbsid) = split( /\s+/, $_ );
    }
    close( FN );
    if( ! open( FN, "$inndhome/nodelist.bbs" ) ) {
	return 0;
    }
    @NODELIST = <FN>;
    close( FN );
    if( ! open( FN, "$inndhome/newsfeeds.bbs" ) ) {
	return 0;
    }
    @NEWSFEEDS = <FN>;
    close( FN );
    return 1;
}

sub search_nodelist
{
    local	($site) = @_;
    local	($id, $addr, $name);

#    print "($site)";

    foreach $line ( @NODELIST ) {
#	chop( $line );
#	print "$line site $site\n";
	($id, $addr, $protocol, $name) = split( /\s+/, $line );
	if (index($id,'/') >= 0) {
	   $id = substr($id,0, index($id,'/'));
	}
#	chop $name;
	if( $id eq $site ) {
	    $name = substr( $line, index( $line, $name ) );
	    chop $name;
#            print "($addr $id $#NODELIST ",length($line),")" ;
	    return( $addr, $protocol, $name );
	}  
    }
    return( $null, $null, $null);
}

sub search_group
{
    local	($newsgroup) = @_;
    local	($group, $board, $server);

    foreach $line ( @NEWSFEEDS ) {
	($group, $board, $server) = split( /\s+/, $line );
	if( $group eq $newsgroup ) {
	    return( $board, $server );
	}
    }
    return( $null, $null );
}

sub search_board
{
    local	($bbsboard) = @_;
    local	($group, $board, $server);

    foreach $line ( @NEWSFEEDS ) {
	($group, $board, $server) = split( /\s+/, $line, 3 );
	if( $board eq $bbsboard && ord( $line ) != ord( "#" ) ) {
	    return( $group, $server );
	}
    }
    return( $null, $null );
}

sub ascii_date
{
    local       ($time) = @_;
    local       ($sec,$min,$hr,$mday,$mon,$year,$wday,$yday,$isdst);

    ($sec,$min,$hr,$mday,$mon,$year,$wday,$yday,$isdst) = gmtime( $time );
    $wday = ("Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat")[ $wday ];
    $mon = ("Jan", "Feb", "Mar", "Apr", "May", "Jun",
            "Jul", "Aug", "Sep", "Oct", "Nov", "Dec")[ $mon ];
    return sprintf( "$mday $mon 19$year %02d:%02d:%02d GMT", $hr, $min, $sec );
}

sub bbslog
{
    if( $logfile ) {
	open( FN, ">> $logfile" );
	print FN @_;
	close( FN );
    }
}

sub get_tmpfile
{
    open( FN, $tmpfile );
    $result = <FN>;
    close( FN );
    unlink( $tmpfile );
    return( $result );
}

sub lock_history
{
    foreach $n (1..10) {
	if( ! -f "$history.lock" ) {
	    last;
	}
	sleep( 1 );
    }
    open( FN, "> $history.lock" );
    close( FN );
}

sub add_history
{
    local	($msgid, $filelist) = @_;
    local	(%HIST);

    &lock_history();

    dbmopen( %HIST, $history, 0666 );
    $HIST{ $msgid } = $filelist;
    dbmclose( HIST );

    open( FN, ">> $history" );
    print FN "$msgid\t$filelist\n";
    close( FN );

    unlink( "$history.lock" );
}

sub find_history
{
    local	($msgid) = @_;
    local	(%HIST, $filelist);

    &lock_history();

    dbmopen( %HIST, $history, 0666 );
    $filelist = $HIST{ $msgid };
    dbmclose( HIST );
    if( ! $filelist ) {
	dbmopen( %HIST, "$history.z", 0666 );
	$filelist = $HIST{ $msgid };
	dbmclose( HIST );
    }

    @STAT = stat( "$history" );
    $time = $STAT[ 9 ];
    @STAT = stat( "$history.z" );
    $ztime = $STAT[ 9 ];
    if( $time - $ztime > 7 * 86400 ) {
	rename( "$history", "$history.z" );
	rename( "$history.dir", "$history.z.dir" );
	rename( "$history.pag", "$history.z.pag" );
    }

    unlink( "$history.lock" );
    return( $filelist );
}

