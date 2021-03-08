import { createApp } from "vue";
import App from "./App.vue";
import router from "./router";
import '@/plugins/bootstrap.js'
import '@/plugins/bootstrap.scss'
import { FontAwesomeIcon } from "./plugins/font-awesome.js"
import Toaster from "@meforma/vue-toaster"
import "@/assets/aha.scss"
import "file-icons-js/css/style.css"
import "@/assets/article.css"

createApp(App)
	.use(router)
	.use(Toaster)
	.component('fa', FontAwesomeIcon)
	.mount("#app");
