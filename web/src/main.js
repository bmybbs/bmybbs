import { createApp } from "vue";
import App from "./App.vue";
import router from "./router";
import 'bootstrap'
import 'bootstrap/dist/css/bootstrap.min.css'
import { FontAwesomeIcon } from "./plugins/font-awesome.js"
import Toaster from "@meforma/vue-toaster"
import "@/assets/aha.scss"
import "file-icons-js/css/style.css"
import "@/assets/article.css"
import "@/plugins/mathjax.js"

createApp(App)
	.use(router)
	.use(Toaster)
	.component('fa', FontAwesomeIcon)
	.mount("#app");
