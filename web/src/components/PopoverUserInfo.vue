<template>
	<span ref="span" @mouseover="openPopover" @mouseout="closePopover">{{ _userid }}</span>
</template>

<script>
import { createApp, defineAsyncComponent } from "vue"
import Popover from "bootstrap/js/dist/popover"
const CardUserInfo = defineAsyncComponent(() => import("./CardUserInfo.vue"));

export default {
	data() {
		return {
			timeout: null,
			mount_id: "usercard" + (new Date().getTime()),
			popover: null,
			v_instance: null,
		};
	},
	props: {
		_userid: String,
	},
	methods: {
		openPopover() {
			this.timeout = setTimeout(() => {
				this.doOpen();
				this.timeout = null;
			}, 1000);
		},
		doOpen() {
			const that = this;

			this.v_instance = createApp(CardUserInfo, {
				_userid: that._userid,
			});

			this.popover = new Popover(this.$refs.span, {
				container: "body",
				placement: "bottom",
				html: true,
				content: "<div id='" + that.mount_id + "'></div>",
			});

			this.popover.show();
			this.v_instance.mount("#" + this.mount_id);
		},
		closePopover() {
			if (this.timeout) {
				clearTimeout(this.timeout);
				this.timeout = null;
			} else {
				if (this.v_instance) {
					this.v_instance.unmount("#" + this.mount_id);
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

<style>
span { display: inline-block; }
</style>

