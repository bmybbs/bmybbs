import { BMYClient } from "../../utils/BMYClient.js"

Component({
	properties: {
		board: { type: Object },
		digest: { type: Boolean, value: false },
	},
	data: { },
	attached() {
		if (this.data.digest) {
			BMYClient.get_board_info(this.data.board.name).then(response => {
				this.setData({ board: response });
			});
		}
	},
	methods: {
		gotoBoard() {
			wx.navigateTo({ url: `../../pages/board/board?boardname=${this.data.board.name}` })
		},
	},
});

