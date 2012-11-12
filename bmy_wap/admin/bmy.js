Ext.onReady(function(){
	
	BLANK_IMAGE_URL ="extjs/resources/images/default/s.gif";
	function dosongsearch()
	{	
		songurl='search.php?id='+Ext.get('user').dom.value+'&ip='+Ext.get('ip').dom.value+'&starttime='+Ext.get('starttime').dom.value+'&stoptime='+Ext.get('stoptime').dom.value+'&content='+Ext.get('content').dom.value;
		
		//alert(songurl);
		//songurl=encodeURI(songurl);
		Ext.get('bmy').dom.innerHTML="正在搜索，请稍候。。。<br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>";
		
		
		var store = new Ext.data.Store({   
        	proxy: new Ext.data.HttpProxy({   
            	url: songurl,
				method:'GET'
      		}),   
			
        	reader: new Ext.data.JsonReader({   
            	root: 'bmy',   
            	totalProperty: 'total',     
            	fields: [
						{name: 'id'},
				    	{name: 'time'},
						{name: 'ip'},
						{name: 'type'},
						{name: 'title'}
				  	]
       		 })  
  
      	 });  
		
		
		var grid = new Ext.grid.GridPanel({
			 store: store,
			 columns: [
					    {id:'id',header: "用户名", dataIndex: 'id'},
						{header: "时间", sortable: true, dataIndex: 'time'},
						{header: "间隔", sortable: true, dataIndex: 'type'},
						{header: "IP", sortable: true, dataIndex: 'ip'},
						{id:'title',header: "标题", dataIndex: 'title'}
						],
						autoExpandColumn: 'title',
						height:500,
						width:860,
					    title:'访问日志',
						loadMask: true,  
						loadMask: {msg: "正在搜索...请稍后..."}
						//bbar: new Ext.PagingToolbar({   
						//	pageSize:50,   
						//	store: store,   
						//	displayInfo: true,   
						//	displayMsg: '显示{0} - {1} of {2}',   
						//	emptyMsg: "没有找到符合要求的" 
					   //})   

		 });
		
		Ext.get('bmy').dom.innerHTML="";
		grid.render('bmy');
		store.load();
		//store.load({params:{start:0,limit:50}});   					
		grid.getSelectionModel().selectFirstRow();		
	}
	
	Ext.get('bmysearch').on('click',dosongsearch);
	
	//Ext.get('songsearch').addKeyListener(13,dosongsearch);
	//document.onkeydown=function() {	
	//		if (event.keyCode==13)
	//				dosongsearch();
	//}
	dosongsearch();

})