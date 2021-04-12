Component({
	properties: {
		isReply:  { type: Boolean, value: false },
		show:     { type: Boolean },
		title:    { type: String },
	},
	data: {
		doClose: false,
		body: "",
	},
	attached() {
	},
	methods: {
		doNothing() {
			// 本方法用于 input / textarea bindinput
			// 启用双向绑定 model:value 方式后不清楚什么原因，在 input / textarea 输入文本
			// 后会在控制台抛出
			// Do not have  handler in component: components/EditorOverlay/EditorOverlay.
			// Please make sure that  handler has been defined in
			// components/EditorOverlay/EditorOverlay.
			// 的警告信息，腾讯的文档实在是 ...
		},
		preventClose() {
			if (!this.data.doClose) {
				this.setData({ show: true });
			}
		},
		closeOverlay() {
			this.setData({ doClose: true, show: false });
		},
		cleanInput() {
			this.setData({ title: "", body: "" });
		},
		post() {
			const obj = {
				isReply: this.data.isReply,
				body: this.data.body,
				onSuccess: () => {
					this.cleanInput();
					this.closeOverlay();
				}
			};

			if (!this.data.isReply) {
				obj.title = this.data.title;
			}

			this.triggerEvent("post", obj, {});
		},
	},
});

