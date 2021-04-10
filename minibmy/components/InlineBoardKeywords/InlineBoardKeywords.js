Component({
	properties: {
		str: { type: String },
	},
	data: {
		keywords: [],
	},
	attached() {
		if (this.data.str && this.data.str.length > 0) {
			this.setData({
				keywords: this.data.str.split(/[\s,;.:]+/)
			});
		}
	},
})

