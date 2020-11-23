<template>
	<LoginMobile v-if="is_mobile" />
	<LoginPC v-else />
</template>

<script>
import LoginPC from "@/components/LoginPC.vue";
import LoginMobile from "@/components/LoginMobile.vue";

export default {
	data() {
		return {
			is_mobile: false,
		};
	},
	created() {
		this.check_width();
		window.addEventListener("resize", this.resize_handler);
	},
	unmounted() {
		window.removeEventListener("resize", this.resize_handler);
	},
	components: {
		LoginPC,
		LoginMobile,
	},
	methods: {
		check_width() {
			this.is_mobile = (window.innerWidth <= 600);
		},
		resize_handler() {
			this.check_width();
		}
	},
};
</script>

<style>
</style>

