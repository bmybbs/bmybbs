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

			<ul class="editor-toolbar nav nav-tabs" v-if="isEditing" ref="editor_toolbar">
				<li class="nav-item" data-bs-toggle="tooltip" data-bs-placement="top" title="附件">
					<span class="nav-link">
						<fa icon="paperclip" />
					</span>
				</li>
				<li class="nav-item dropdown" data-bs-toggle="tooltip" data-bs-placement="top" title="前景色">
					<span class="nav-link" @click="openFcdd">
						<fa icon="font" />
					</span>
					<ul class="dropdown-menu" :class="{ show: showFcdd }">
						<li>
							<div class="palette">
								<span class="aha aha-bg-black" @click="insertFcBlack"></span>
								<span class="aha aha-bg-red" @click="insertFcRed"></span>
								<span class="aha aha-bg-green" @click="insertFcGreen"></span>
								<span class="aha aha-bg-olive" @click="insertFcOlive"></span>
							</div>
							<div class="palette">
								<span class="aha aha-bg-blue" @click="insertFcBlue"></span>
								<span class="aha aha-bg-purple" @click="insertFcPurple"></span>
								<span class="aha aha-bg-teal" @click="insertFcTeal"></span>
								<span class="aha aha-bg-grey" @click="insertFcGrey"></span>
							</div>
						</li>
					</ul>
				</li>
				<li class="nav-item dropdown" data-bs-toggle="tooltip" data-bs-placement="top" title="背景色">
					<span class="nav-link" @click="openBgdd">
						<fa icon="clone" />
					</span>
					<ul class="dropdown-menu" :class="{ show: showBgdd }">
						<li>
							<div class="palette">
								<span class="aha aha-bg-black" @click="insertBgBlack"></span>
								<span class="aha aha-bg-red" @click="insertBgRed"></span>
								<span class="aha aha-bg-green" @click="insertBgGreen"></span>
								<span class="aha aha-bg-olive" @click="insertBgOlive"></span>
							</div>
							<div class="palette">
								<span class="aha aha-bg-blue" @click="insertBgBlue"></span>
								<span class="aha aha-bg-purple" @click="insertBgPurple"></span>
								<span class="aha aha-bg-teal" @click="insertBgTeal"></span>
								<span class="aha aha-bg-grey" @click="insertBgGrey"></span>
							</div>
						</li>
					</ul>
				</li>
				<li class="nav-item" data-bs-toggle="tooltip" data-bs-placement="top" title="闪烁">
					<span class="nav-link" @click="insertBlink">
						<fa icon="lightbulb" />
					</span>
				</li>
				<li class="nav-item" data-bs-toggle="tooltip" data-bs-placement="top" title="斜体">
					<span class="nav-link" @click="insertItalic">
						<fa icon="italic" />
					</span>
				</li>
				<li class="nav-item" data-bs-toggle="tooltip" data-bs-placement="top" title="下划线">
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
import Tooltip from "bootstrap/js/dist/tooltip"
import { BMYClient } from "@/lib/BMYClient.js"
import { ANSI_TAGS } from "@/lib/BMYConstants.js"

export default {
	data() {
		return {
			isEditing: true,
			previewContent: "",
			fc_dd: null,
			showFcdd: false,
			showBgdd: false,
		};
	},
	mounted() {
		let elements = this.$refs.editor_toolbar.querySelectorAll('[data-bs-toggle="tooltip"]');
		let tooltipTriggerList = [].slice.call(elements);
		tooltipTriggerList.map((el) => {
			new Tooltip(el);
		});
	},
	methods: {
		openFcdd() {
			this.showBgdd = false;
			this.showFcdd = true;
		},
		openBgdd() {
			this.showFcdd = false;
			this.showBgdd = true;
		},
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
			this.showFcdd = false;
			this.showBgdd = false;
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
		insertBlink()    { this.insertAtCursor(ANSI_TAGS.ANSI_BLINK,     ANSI_TAGS.ANSI_CLEAR); },
		insertItalic()   { this.insertAtCursor(ANSI_TAGS.ANSI_ITALIC,    ANSI_TAGS.ANSI_CLEAR); },
		insertUnderl()   { this.insertAtCursor(ANSI_TAGS.ANSI_UNDERL,    ANSI_TAGS.ANSI_CLEAR); },
		insertFcBlack()  { this.insertAtCursor(ANSI_TAGS.ANSI_FC_BLACK,  ANSI_TAGS.ANSI_CLEAR); },
		insertFcRed()    { this.insertAtCursor(ANSI_TAGS.ANSI_FC_RED,    ANSI_TAGS.ANSI_CLEAR); },
		insertFcGreen()  { this.insertAtCursor(ANSI_TAGS.ANSI_FC_GREEN,  ANSI_TAGS.ANSI_CLEAR); },
		insertFcOlive()  { this.insertAtCursor(ANSI_TAGS.ANSI_FC_YELLOW, ANSI_TAGS.ANSI_CLEAR); },
		insertFcBlue()   { this.insertAtCursor(ANSI_TAGS.ANSI_FC_BLUE,   ANSI_TAGS.ANSI_CLEAR); },
		insertFcPurple() { this.insertAtCursor(ANSI_TAGS.ANSI_FC_PINK,   ANSI_TAGS.ANSI_CLEAR); },
		insertFcTeal()   { this.insertAtCursor(ANSI_TAGS.ANSI_FC_CYAN,   ANSI_TAGS.ANSI_CLEAR); },
		insertFcGrey()   { this.insertAtCursor(ANSI_TAGS.ANSI_FC_WHITE,  ANSI_TAGS.ANSI_CLEAR); },
		insertBgBlack()  { this.insertAtCursor(ANSI_TAGS.ANSI_BG_BLACK,  ANSI_TAGS.ANSI_CLEAR); },
		insertBgRed()    { this.insertAtCursor(ANSI_TAGS.ANSI_BG_RED,    ANSI_TAGS.ANSI_CLEAR); },
		insertBgGreen()  { this.insertAtCursor(ANSI_TAGS.ANSI_BG_GREEN,  ANSI_TAGS.ANSI_CLEAR); },
		insertBgOlive()  { this.insertAtCursor(ANSI_TAGS.ANSI_BG_YELLOW, ANSI_TAGS.ANSI_CLEAR); },
		insertBgBlue()   { this.insertAtCursor(ANSI_TAGS.ANSI_BG_BLUE,   ANSI_TAGS.ANSI_CLEAR); },
		insertBgPurple() { this.insertAtCursor(ANSI_TAGS.ANSI_BG_PINK,   ANSI_TAGS.ANSI_CLEAR); },
		insertBgTeal()   { this.insertAtCursor(ANSI_TAGS.ANSI_BG_CYAN,   ANSI_TAGS.ANSI_CLEAR); },
		insertBgGrey()   { this.insertAtCursor(ANSI_TAGS.ANSI_BG_WHITE,  ANSI_TAGS.ANSI_CLEAR); },
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

.editor-toolbar .nav-item:hover {
	background-color: #ccc;
}

.editor-toolbar .dropdown-menu {
	background: #ddd;
}

.palette {
	padding-left: 20px;
}

.palette span {
	display: inline-block;
	width: 20px;
	height: 20px;
}

.palette span:not(:first-child) {
	margin-left: 5px;
}
</style>

