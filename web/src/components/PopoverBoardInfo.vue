<template>
	<span ref="span" @mouseover="openPopover" @mouseout="closePopover" @click="closePopover"><router-link :to="{ name: 'board', params: { boardname: _boardname_en }}" class="text-decoration-none">{{_boardname_zh}}</router-link></span>
</template>

<script>
import { createApp, defineAsyncComponent } from "vue"
import Popover from "bootstrap/js/dist/popover"
import { BMYClient } from "@/lib/BMYClient.js"
const CardBoardInfo = defineAsyncComponent(() => import("./CardBoardInfo.vue"));

export default {
	data() {
		return {
			timeout: null,
			internal_id: (new Date().getTime()),
			status_mouse_on: false,
			info: null,
			popover: null,
			v_instance: null,
		};
	},
	props: {
		_boardname_en: String,
		_boardname_zh: String,
	},
	methods: {
		openPopover() {
			if (!this.status_mouse_on) {
				this.timeout = setTimeout(() => {
					this.status_mouse_on = true;
					if (this.info == null) {
						BMYClient.get_board_info(this._boardname_en).then(response => {
							this.info = response;
							this.doOpen();
						});
					} else {
						this.doOpen();
					}
					this.timeout = null;
				}, 1000);
			}
		},
		doOpen() {
			var that = this;

			this.v_instance = createApp(CardBoardInfo, {
				_boardname_en: that._boardname_en,
				_boardname_zh: that._boardname_zh,
				_secstr: that.info.secstr,
				_article_num: that.info.article_num,
				_thread_num: that.info.thread_num,
				_inboard_num: that.info.inboard_num,
				_today_new: that.info.today_new,
			});

			this.popover = new Popover(this.$refs.span, {
				container: "body",
				placement: "bottom",
				html: true,
				content: "<div id='card" + that.internal_id + "'></div>"
			});

			this.popover.show();
			this.v_instance.mount("#card" + this.internal_id);
		},
		closePopover() {
			this.status_mouse_on = false;
			if (this.timeout) {
				clearTimeout(this.timeout);
				this.timeout = null;
			} else {
				if (this.v_instance) {
					this.v_instance.unmount("#card" + this.internal_id);
					this.v_instance = null;
				}

				if (this.popover) {
					this.popover.dispose();
					this.popover = null;
				}
			}
		},
	},
}
</script>

<style scoped>
span { display: inline-block; }
</style>

