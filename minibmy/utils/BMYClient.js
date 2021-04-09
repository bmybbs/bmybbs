import { BASE_URL } from "BMYConstants.js"

let cookie_str = "";

const myFetchGet = (url) => {
	return new Promise((resolve) => {
		wx.request({
			url: BASE_URL + url,
			header: {
				"cookie": cookie_str
			},
			success(res) {
				resolve(res.data);
			}
		});
	});
};

const myFetchPost = (url) => {
	return new Promise((resolve) => {
		wx.request({
			url: BASE_URL + url,
			method: "POST",
			header: {
				"cookie": cookie_str
			},
			success(res) {
				resolve(res.data);
				if (res.cookies.length > 0)
					cookie_str = res.cookies[0];
			}
		});
	});
};

export const BMYClient = {
	get_announce() {
		return myFetchGet("/api/article/list?type=announce");
	},
	get_article_content(boardname_en, aid) {
		return myFetchGet(`/api/article/getContent?board=${boardname_en}&aid=${aid}`);
	},
	get_article_list_by_board(boardname_en, mode, page = 1) {
		return myFetchGet(`/api/article/list?type=board&board=${boardname_en}&btype=${mode}&page=${page}`);
	},
	get_article_list_by_section(secstr, page = 1) {
		return myFetchGet(`/api/article/list?type=section&secstr=${secstr}&page=${page}`);
	},
	get_board_info(boardname_en) {
		return myFetchGet(`/api/board/info?bname=${boardname_en}`);
	},
	get_boards_by_section(secstr, sortmode) {
		return myFetchGet(`/api/board/list?secstr=${secstr}&sortmode=${sortmode}`);
	},
	get_commend() {
		return myFetchGet("/api/article/list?type=commend");
	},
	get_fav_board_list() {
		return myFetchGet("/api/board/fav/list");
	},
	get_feed(page = 1) {
		return myFetchGet(`/api/subscription/list?page=${page}`);
	},
	get_sectop(secstr) {
		return myFetchGet(`/api/article/list?type=sectop&secstr=${secstr}`);
	},
	get_thread_list(boardname_en, tid) {
		return myFetchGet(`/api/article/list?type=thread&board=${boardname_en}&thread=${tid}`);
	},
	get_top10() {
		return myFetchGet("/api/article/list?type=top10");
	},
	get_user_info(userid) {
		return myFetchGet("/api/user/query?queryid=" + userid);
	},
	oauth_get_code(code, tfakey) {
		return myFetchPost(`/api/oauth/2fa_get_code?code=${code}&tfakey=${tfakey}`);
	},
	oauth_login(code) {
		return myFetchPost(`/api/oauth/login?code=${code}`);
	},
	search_board(start_with) {
		return myFetchGet(`/api/board/autocomplete?search_str=${start_with}`);
	},
	search_user(start_with) {
		return myFetchGet(`/api/user/autocomplete?search_str=${start_with}`);
	},
}

