Component({
	properties: {
		item: {
			type: Object
		}
	},
	methods: {
		gotoSection() {
			wx.navigateTo({
				url: `../section/section?id=${this.data.item.id}`
			});
		}
	}
});

