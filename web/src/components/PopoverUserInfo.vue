<template>
	<span ref="span" @mouseover="openPopover" @mouseout="closePopover">{{ _userid }}</span>
</template>

<script>
import { createApp } from "vue"
import Popover from "bootstrap/js/dist/popover"
import { BMYClient } from "@/lib/BMYClient.js"
import CardUserInfo from "@/components/CardUserInfo.vue"

export default {
	data() {
		return {
			mount_id: "usercard" + (new Date().getTime()),
			info: null,
			popover: null,
			v_instance: null,
		};
	},
	props: {
		_userid: String,
	},
	methods: {
		openPopover() {
			if (this.info == null) {
				BMYClient.get_user_info(this._userid).then(response => {
					this.info = response;
					this.doOpen();
				});
			} else {
				this.doOpen();
			}
		},
		doOpen() {
			var that = this;

			this.v_instance = createApp(CardUserInfo, {
				_userid: that.info.userid,
				_exp_level: that.info.exp_level,
				_job: that.info.job,
				_perf_level: that.info.perf_level,
				_login_counts: that.info.login_counts,
				_post_counts: that.info.post_counts,
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
			if (this.v_instance) {
				this.v_instance.unmount("#" + this.mount_id);
				this.v_instance = null;
			}

			if (this.popover) {
				this.popover.dispose();
				this.popover = null;
			}
		},
	},
}
</script>

<style>
span { display: inline-block; }
</style>

