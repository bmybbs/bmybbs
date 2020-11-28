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
};

