<template>
	<div class="row">
		<div class="col-sm-4 order-sm-5 col-xxl-3 col-xs-12">
			<CardBoardInfo
				v-bind:_boardname_en="$route.params.boardname"
				v-bind:_events="{}"
			/>
		</div>
		<div class="col-sm-8 col-xxl-9 col-xs-12">
			<router-view />
		</div>
	</div>
</template>

<script>
import { defineAsyncComponent } from "vue"
const CardBoardInfo = defineAsyncComponent(() => import("@/components/CardBoardInfo.vue"));

export default {
	data() {
		return {
		};
	},
	components: {
		CardBoardInfo,
	},
}
</script>

<style scoped>
</style>

