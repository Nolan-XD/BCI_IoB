/*
 * @Author: Nolan-XD haopengliuu@163.com
 * @Date: 2025-05-28 02:09:32
 * @LastEditors: Nolan-XD haopengliuu@163.com
 * @LastEditTime: 2025-07-22 10:48:17
 * @FilePath: \BCI_IoB\iot-platform-demo-main\iot-platform-demo-main\iot-demo-docker\frontend\vite.config.js
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
// vite.config.js
import { defineConfig } from 'vite'
import vue from '@vitejs/plugin-vue'

export default defineConfig({
  plugins: [vue()],
  server: {
    host: '0.0.0.0',  // 允许外部访问
    port: 3000,
    watch: {
      usePolling: true,  // 启用轮询监听
      interval: 100      // 轮询间隔，单位ms，100毫秒
    }
  },
  define: {
    __VUE_PROD_DEVTOOLS__: false,       // 关闭开发工具
    __VUE_OPTIONS_API__: true,           // 启用选项式API
    __VUE_PROD_HYDRATION_MISMATCH_DETAILS__: 'false'  // 根据需求设置 true/false	  
  }
})
