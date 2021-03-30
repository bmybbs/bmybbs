<template>
	<div>
		<TabbedEditor />
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

