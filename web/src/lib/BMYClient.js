function myFetchGet(url) {
	return fetch(url).then(response => response.json());
}

export const BMYClient = {
	get_announce() {
		return myFetchGet("/api/article/list?type=announce");
	},
	get_board_info(boardname_en) {
		return myFetchGet("/api/board/info?bname=" + boardname_en);
	},
	get_boards_by_section(secstr, sortmode) {
		return myFetchGet("/api/board/list?secstr=" + secstr + "&sortmode=" + sortmode);
	},
	get_commend() {
		return myFetchGet("/api/article/list?type=commend");
	},
	get_fav_board_list() {
		return myFetchGet("/api/board/fav/list");
	},
	get_feed(start) {
		return myFetchGet("/api/subscription/list?start=" + start);
	},
	get_user_info(userid) {
		return myFetchGet("/api/user/query?queryid=" + userid);
	}
};

