<template>
	<div>
		<div class="editor-header">
			<ul class="nav nav-tabs">
				<li class="nav-item">
					<span class="nav-link" :class="{ active: isEditing }" @click="turnOnEdit">编辑</span>
				</li>
				<li class="nav-item">
					<span class="nav-link" :class="{ active: isAttach }" @click="turnOnAttach">附件</span>
				</li>
				<li class="nav-item">
					<span class="nav-link" :class="{ active: !isEditing && !isAttach }" @click="turnOnPreview">预览</span>
				</li>
			</ul>

			<ul class="editor-toolbar nav nav-tabs" v-if="isEditing" ref="editor_toolbar">
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

			<ul class="editor-toolbar nav nav-tabs" v-if="isAttach" ref="attach_toolbar">
				<li class="nav-item" data-bs-toggle="tooltip" data-bs-placement="top" title="选择文件">
					<span class="nav-link" @click="pickFiles">
						<fa icon="paperclip" />
					</span>
				</li>
				<li class="nav-item" data-bs-toggle="tooltip" data-bs-placement="top" title="刷新">
					<span class="nav-link" @click="loadUploaded">
						<fa icon="redo" />
					</span>
				</li>
				<li class="nav-item" data-bs-toggle="tooltip" data-bs-placement="top" title="上传">
					<span class="nav-link">
						<fa icon="cloud-upload-alt" />
					</span>
				</li>
				<li class="nav-item" data-bs-toggle="tooltip" data-bs-placement="top" title="参考代码">
					<span class="nav-link">
						<fa icon="code" />
					</span>
				</li>
			</ul>
		</div>
		<div class="tab-content">
			<div class="tab-pane fade" :class="{ active: isEditing, show: isEditing }">
				<textarea ref="textarea" class="form-control" rows="5"></textarea>
				<button class="btn btn-primary">回复</button>
			</div>

			<div class="tab-pane fade" :class="{ active: isAttach, show: isAttach, isDragging: isDragging }"
				ref="dropbox"
				@dragenter.stop.prevent="dragEnter"
				@dragleave.stop.prevent="dragLeave"
				@dragover.stop.prevent
				@drop.prevent="dropFiles">
				<input ref="filepicker" type=file style="display:none" multiple @change="handleFiles">
				<div v-if="files.length == 0" class="text-center py-5">
					当前没有附件，可以点击回纹针添加文件或者拖拽文件到这里。
				</div>
				<div v-else>
					<div v-for="file in files" :key="file.name" class="d-flex flex-row">
						<div class="upload-icon p-2" :class="{ uploaded: file.status.uploaded }">
							<fa icon="file" />
						</div>
						<div class="d-flex flex-column align-self-center">
							<div class="upload-h1">
								{{file.name}}
							</div>
							<div class="upload-meta">
								<span>{{file.size}}</span>
								<span v-if="file.status.uploaded">已上传</span>
								<span v-if="file.status.pending">待上传</span>
								<span v-if="file.status.hasError">错误</span>
							</div>
						</div>
					</div>
				</div>
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
import { readableSize } from "@bmybbs/bmybbs-content-parser/dist/utils.js"

export default {
	data() {
		return {
			isEditing: true,
			isAttach: false,
			isDragging: false,
			previewContent: "",
			fc_dd: null,
			showFcdd: false,
			showBgdd: false,
			uploadedFiles: [],
			pendingFiles: [],
			files: [],
		};
	},
	mounted() {
		let elements = this.$refs.editor_toolbar.querySelectorAll('[data-bs-toggle="tooltip"]');
		let tooltipTriggerList = [].slice.call(elements);
		tooltipTriggerList.forEach((el) => {
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
			this.isAttach = false;
		},
		turnOnAttach() {
			this.isEditing = false;
			this.isAttach = true;
			this.loadUploaded();
		},
		turnOnPreview() {
			this.isEditing = false;
			this.isAttach = false;

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
		loadUploaded() {
			BMYClient.get_attach_list().then(response => {
				if (response.errcode == 0) {
					this.uploadedFiles = response.attach_array;
					this.updateFileList();
				}
			});
		},
		dragEnter() {
			this.isDragging = true;
		},
		dragLeave() {
			this.isDragging = false;
		},
		dropFiles(event) {
			this.isDragging = false;
			this.pendingFiles = [ ...event.dataTransfer.files ];
			this.updateFileList();
		},
		pickFiles() {
			this.$refs.filepicker.click();
		},
		handleFiles() {
			this.pendingFiles = [ ...this.$refs.filepicker.files ];
			this.updateFileList();
		},
		updateFileList() {
			const arr = [];

			this.uploadedFiles.forEach(file => {
				arr.push({
					name: file.file_name,
					size: readableSize(file.size),
					status: {
						uploaded: true,
						pending: false,
						hasError: false,
					},
				});
			});

			this.pendingFiles.forEach(file => {
				arr.push({
					name: file.name,
					size: readableSize(file.size),
					status: {
						uploaded: false,
						pending: true,
						hasError: false,
					},
				});
			});

			this.files = arr;
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

.isDragging {
	border: 10px solid #00ff00;
}

.upload-icon {
	font-size: 220%;
	color: #638ade;
}

.upload-icon.uploaded {
	color: #62bf71;
}
</style>

