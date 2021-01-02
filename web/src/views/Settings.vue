<template>
	<div>
		<div class="accordion" v-if="loggedin">
			<AccordionSettingWechat />
		</div>
		<div v-else>
			请先登录
		</div>
	</div>
</template>

<script>
import { BMYClient } from "@/lib/BMYClient.js"
import { BMY_EC } from "@/lib/BMYConstants.js"
import AccordionSettingWechat from "@/components/AccordionSettingWechat.vue"

export default {
	data() {
		return {
			loggedin: true,
		};
	},
	mounted() {
		BMYClient.user_check().then(response => {
			if (response.code != BMY_EC.API_RT_SUCCESSFUL) {
				this.loggedin = false;
			}
		});
	},
	components: {
		AccordionSettingWechat,
	},
}
</script>

<style scoped>
.accordion >>> .accordion-item > div {
	padding: 16px 20px;
}
</style>

