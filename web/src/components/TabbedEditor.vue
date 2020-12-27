<template>
	<div>
		<div class="editor-header">
			<ul class="nav nav-tabs">
				<li class="nav-item">
					<span class="nav-link" :class="{ active: isEditing }" @click="turnOnEdit">编辑</span>
				</li>
				<li class="nav-item">
					<span class="nav-link" :class="{ active: !isEditing }" @click="turnOnPreview">预览</span>
				</li>
			</ul>

			<ul class="editor-toolbar nav nav-tabs" v-if="isEditing">
				<li class="nav-item">
					<span class="nav-link">
						<fa icon="paperclip" />
					</span>
				</li>
				<li class="nav-item">
					<span class="nav-link">
						<fa icon="font" />
					</span>
				</li>
				<li class="nav-item">
					<span class="nav-link">
						<fa icon="clone" />
					</span>
				</li>
				<li class="nav-item">
					<span class="nav-link" @click="insertBlink">
						<fa icon="lightbulb" />
					</span>
				</li>
				<li class="nav-item">
					<span class="nav-link" @click="insertItalic">
						<fa icon="italic" />
					</span>
				</li>
				<li class="nav-item">
					<span class="nav-link" @click="insertUnderl">
						<fa icon="underline" />
					</span>
				</li>
			</ul>
		</div>
		<div class="tab-content">
			<div class="tab-pane fade" :class="{ active: isEditing, show: isEditing }">
				<textarea ref="textarea" class="form-control" rows="5"></textarea>
				<button class="btn btn-primary">回复</button>
			</div>

			<div class="tab-pane fade" :class="{ active: !isEditing, show: !isEditing }" v-html="previewContent">
			</div>
		</div>
	</div>
</template>

<script>
import { BMYClient } from "@/lib/BMYClient.js"
import { ANSI_TAGS } from "@/lib/BMYConstants.js"

export default {
	data() {
		return {
			isEditing: true,
			previewContent: "",
		};
	},
	methods: {
		turnOnEdit() {
			this.isEditing = true;
		},
		turnOnPreview() {
			this.isEditing = false;

			BMYClient.get_draft_preview({
				content: this.$refs.textarea.value,
			}).then(response => {
				if (response.errcode == 0) {
					this.previewContent = response.content;
				} else {
					this.previewContent = "";
				}
			});
		},
		insertAtCursor(left, right) {
			let textarea = this.$refs.textarea;
			if (textarea.selectionStart || textarea.selectionStart == '0') {
				let startPos = textarea.selectionStart,
					endPos = textarea.selectionEnd,
					draft = textarea.value;

				textarea.value = [
					draft.substring(0, startPos),
					left,
					right,
					draft.substring(endPos, draft.length)
				].join("");
				textarea.focus();
				textarea.selectionEnd = startPos + left.length;
			} else {
				let pos = this.$refs.value.length + left.length;
				textarea.value += left + right;
				textarea.focus();
				textarea.selectionEnd = pos;
			}
		},
		insertBlink() { this.insertAtCursor(ANSI_TAGS.ANSI_BLINK, ANSI_TAGS.ANSI_CLEAR); },
		insertItalic() { this.insertAtCursor(ANSI_TAGS.ANSI_ITALIC, ANSI_TAGS.ANSI_CLEAR); },
		insertUnderl() { this.insertAtCursor(ANSI_TAGS.ANSI_UNDERL, ANSI_TAGS.ANSI_CLEAR); },
	},
}
</script>

<style scoped>
.editor-header {
	display: flex;
	justify-content: space-between;
}

.nav-link:hover {
	cursor: pointer;
}

.tab-content {
	background-color: #fff;
}

textarea {
	font-family: consolas, Lucida Console, Courier New, Courier, monospace;
}
</style>

