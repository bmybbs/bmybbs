<template>
	<span ref="span" @mouseenter="openPopover" @mouseout="closePopover" @click="closePopover"><router-link :to="{ name: 'board', params: { boardname: _boardname_en }}" class="text-decoration-none">{{_boardname_zh}}</router-link></span>
</template>

<script>
import { createApp, defineAsyncComponent } from "vue"
import Popover from "bootstrap/js/dist/popover"
import { FontAwesomeIcon } from "@/plugins/font-awesome.js"
const CardBoardInfo = defineAsyncComponent(() => import("./CardBoardInfo.vue"));

const TIMEOUT_INTERVAL = 1000;

export default {
	data() {
		return {
			timeout_mouse_on: null,
			timeout_mouse_out: null,
			internal_id: (new Date().getTime()),
			status_mouse_on: false,
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
			if (this.timeout_mouse_out) {
				clearTimeout(this.timeout_mouse_out);
				this.timeout_mouse_out = null;
			}

			if (!this.status_mouse_on) {
				this.timeout_mouse_on = setTimeout(() => {
					this.status_mouse_on = true;
					this.doOpen();
					this.timeout_mouse_on = null;
				}, TIMEOUT_INTERVAL);
			}
		},
		doOpen() {
			var that = this;

			this.v_instance = createApp(CardBoardInfo, {
				_boardname_en: that._boardname_en,
				_prevent_events: true,
				_in_popover: true,
				_events: {
					mouseover() {
						if (that.timeout_mouse_out) {
							clearTimeout(that.timeout_mouse_out);
							that.timeout_mouse_out = null;
						}
					},
					mouseout() {
						that.closePopover();
					},
				},
			}).component('fa', FontAwesomeIcon);

			this.popover = new Popover(this.$refs.span, {
				container: "body",
				placement: "bottom",
				html: true,
				customClass: "bmy-popover-card shadow-lg",
				content: "<div id='card" + that.internal_id + "'></div>"
			});

			this.popover.show();
			this.v_instance.mount("#card" + this.internal_id);
		},
		closePopover() {
			if (this.timeout_mouse_on) {
				clearTimeout(this.timeout_mouse_on);
				this.timeout_mouse_on = null;
				this.status_mouse_on = false;
			} else {
				if (!this.timeout_mouse_out) {
					this.timeout_mouse_out = setTimeout(() => {
						if (this.v_instance) {
							this.v_instance.unmount("#card" + this.internal_id);
							this.v_instance = null;
						}

						if (this.popover) {
							this.popover.dispose();
							this.popover = null;
						}
						this.timeout_mouse_out = null;
						this.status_mouse_on = false;
					}, TIMEOUT_INTERVAL);
				}
			}
		},
	},
}
</script>

<style scoped>
span { display: inline-block; }
</style>

<style>
.bmy-popover-card {
	max-width: 352px;
}
</style>

