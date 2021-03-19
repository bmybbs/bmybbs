<template>
	<div>
		<TabbedEditor :_boardname_en="$route.params.boardname" />
	</div>
</template>

<script>
import { defineAsyncComponent } from "vue"
const TabbedEditor = defineAsyncComponent(() => import("@/components/TabbedEditor.vue"));

export default {
	data() {
		return {
		};
	},
	components: {
		TabbedEditor,
	}
}
</script>

<style scoped>
</style>

