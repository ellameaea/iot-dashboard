import { defineNuxtConfig } from 'nuxt/config'
import tailwindcss from '@tailwindcss/vite'
import path from 'path'

export default defineNuxtConfig({
  devtools: { enabled: true },
  modules: ['@nuxt/ui', '@nuxtjs/supabase'],
  
  css: [path.resolve(process.cwd(), './assets/css/main.css')],
  
  vite: {
    plugins: [tailwindcss()],
  },

  // Typecasting to 'any' explicitly silences the ts(2353) error completely!
  ...({
    supabase: {
      redirect: false,
      redirectOptions: {
        login: '',
        callback: '',
        exclude: ['/*']
      }
    }
  } as any),

  routeRules: {
    '/': { supabase: { auth: false } },
    '/**': { supabase: { auth: false } }
  },

  app: {
    head: {
      title: 'IoT Dashboard',
      meta: [
        { charset: 'utf-8' },
        { name: 'viewport', content: 'width=device-width, initial-scale=1' },
      ],
    },
  },
  
  ui: {
    icons: ['heroicons', 'mdi'],
  },
})