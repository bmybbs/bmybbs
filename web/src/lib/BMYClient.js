function myFetchGet(url) {
	return fetch(url).then(response => response.json());
}

export const BMYClient = {
	get_announce() {
		return myFetchGet("/api/article/list?type=announce");
	},
	get_commend() {
		return myFetchGet("/api/article/list?type=commend");
	},
	get_fav_boards() {
		return myFetchGet("/api/board/fav/list");
	},
	get_feed(start) {
		return myFetchGet("/api/subscription/list?start=" + start);
	},
};

