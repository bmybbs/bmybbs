function myFetchGet(url) {
	return fetch(url).then(response => response.json());
}

function myFetchPost(url, obj) {
	return fetch(url, {
		method: "POST",
		headers: {
			"Content-Type": "application/json"
		},
		body: (obj != null) ? JSON.stringify(obj) : ""
	}).then(response => response.json());
}

export const BMYClient = {
	delete_attach(filename) {
		return fetch(`/api/attach/delete?file=${filename}`, { method: "DELETE" }).then(response => response.json());
	},
	fav_add(boardname_en) {
		return myFetchPost(`/api/board/fav/add?board=${boardname_en}`);
	},
	fav_del(boardname_en) {
		return myFetchPost(`/api/board/fav/del?board=${boardname_en}`);
	},
	get_announce() {
		return myFetchGet("/api/article/list?type=announce");
	},
	get_article_content(boardname_en, aid) {
		return myFetchGet(`/api/article/getContent?board=${boardname_en}&aid=${aid}`);
	},
	get_article_list_by_board(boardname_en, mode) {
		return myFetchGet(`/api/article/list?type=board&board=${boardname_en}&btype=${mode}`);
	},
	get_article_list_by_section(secstr) {
		return myFetchGet(`/api/article/list?type=section&secstr=${secstr}`);
	},
	get_attach_list() {
		return myFetchGet("/api/attach/list");
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
	get_draft_preview(obj) {
		return myFetchPost("/api/article/preview", obj);
	},
	get_fav_board_list() {
		return myFetchGet("/api/board/fav/list");
	},
	get_feed(start) {
		return myFetchGet(`/api/subscription/list?start=${start}`);
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
		return myFetchGet(`/api/user/query?queryid=${userid}`);
	},
	oauth_check_code(code) {
		return myFetchPost(`/api/oauth/2fa_check_code?code=${code}`);
	},
	oauth_get_key() {
		return myFetchGet("/api/oauth/2fa_get_key");
	},
	oauth_remove_wx() {
		return myFetchPost("/api/oauth/remove_wx");
	},
	post_article(article) {
		return myFetchPost("/api/article/post", article);
	},
	search_board(start_with) {
		return myFetchGet(`/api/board/autocomplete?search_str=${start_with}`);
	},
	search_user(start_with) {
		return myFetchGet(`/api/user/autocomplete?search_str=${start_with}`);
	},
	async upload_attach(file) {
		const formData = new FormData();
		formData.append("file", file);
		const response = await fetch("/api/attach/upload", { method: "POST", body: formData });
		if (!response.ok) {
			throw new Error(`HTTP error! status: ${response.status}`);
		} else {
			return await response.json();
		}
	},
	user_check() {
		return myFetchGet("/BMY/user_check");
	},
	user_login(userid, passwd) {
		return myFetchPost("/api/user/login", { userid: userid, passwd: passwd });
	},
	user_logout() {
		return myFetchPost("/api/user/logout");
	},
};

