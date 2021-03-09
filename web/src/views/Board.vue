<template>
	<div class="row">
		<div class="col-sm-4 order-sm-5 col-xxl-3 col-xs-12" v-if="info">
			<CardBoardInfo
				v-bind:_boardname_en="$route.params.boardname"
				v-bind:_boardname_zh="info.zh_name"
				v-bind:_secstr="info.secstr"
				v-bind:_article_num="info.article_num"
				v-bind:_thread_num="info.thread_num"
				v-bind:_inboard_num="info.inboard_num"
				v-bind:_today_new="info.today_new"
			/>
		</div>
		<div class="col-sm-8 col-xxl-9 col-xs-12">
			<router-view />
		</div>
	</div>
</template>

<script>
import { defineAsyncComponent } from "vue"
import { BMYClient } from "@/lib/BMYClient.js"
const CardBoardInfo = defineAsyncComponent(() => import("@/components/CardBoardInfo.vue"));

export default {
	data() {
		return {
			info: null,
		};
	},
	created() {
		this.$watch(() => this.$route.params, (toParams) => {
			this.get_info(toParams.boardname);
		});
	},
	mounted() {
		this.get_info(this.$route.params.boardname);
	},
	methods: {
		get_info(boardname) {
			BMYClient.get_board_info(boardname).then(response => {
				this.info = response;
			});
		},
	},
	components: {
		CardBoardInfo,
	},
}
</script>

<style scoped>
</style>

