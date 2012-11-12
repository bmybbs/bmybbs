var pause=false;

function showloginpics(pics){
    var list=pics.split(";;");
    
    for(i=0;i<list.length;++i){
        // insert pics
		var pic=list[i].split(";");
		$('<a href="'+ pic[1] +'" target="_blank"><img id="pic' + i + '" src="' + pic[0] + '"/></a>').appendTo('div#container');
        if(list.length>1){
        //insert nav-buttons
        $('<a id="nav-button-'+i+'">'+i+'</a>').appendTo('div#nav');
        //nav-button click
        $('a#nav-button-'+i).click(function(){
            var picindex = $(this).text();
            $('#nav a.nav-active').removeClass('nav-active');
            $('a#nav-button-'+picindex).addClass('nav-active');
            
            var targetPic=$('img#pic'+picindex).parent();
            var currentPic=$('#container a.show');
            
            targetPic.css({opacity: 0.0}).addClass('show').animate({opacity:1.0}, 1200);
            currentPic.animate({opacity: 0.0}, 1200).removeClass('show');
        });}
	}
    
	if(list.length>1){	
        $('#nav').width(17*list.length+5);
        $('#nav').css("margin-left",(780-17*list.length-5)/2);
        
        $('img#pic0').parent().addClass('show');
        $('a#nav-button-0').addClass('nav-active');
        
        $('#container a').css({opacity: 0.0});
        
        $('#container a.show').css({opacity: 1.0});
        
        // 设置暂停
        $('#container a').mouseover(function(){
            pause=true;
        });
        $('#container a').mouseout(function(){
            pause=false;
        });
        $('#nav a').mouseover(function(){
            pause=true;
        });
        $('#nav a').mouseout(function(){
            pause=false;
        });
        
        setInterval('gallery()', 6000);
    }
}

function gallery() {
    if(pause==false){
        var current = ($('#container a.show')? $('#container a.show') : $('#container a:first'));

        var next = ((current.next().length)? current.next() : $('#container a:first'));
        
        var currentButton = ($('#nav a.nav-active')? $('#nav a.nav-active') : $('#nav a:first'));
        
        var nextButton = ((currentButton.next().length)? currentButton.next() : $('#nav a:first'));
        
        next.css({opacity: 0.0}).addClass('show').animate({opacity:1.0}, 1200);
        
        nextButton.addClass('nav-active');
        currentButton.removeClass('nav-active');
        
        current.animate({opacity: 0.0}, 1200).removeClass('show');
    }
}