<template>
  <header class="bg-card border-b border-border sticky top-0 z-50">
    <div class="max-w-full mx-auto px-6 py-4">
      <div class="flex items-center justify-between mb-0">
        <div class="flex items-center gap-3">
          <div class="w-10 h-10 bg-primary rounded-lg flex items-center justify-center">
            <UIcon name="i-heroicons-signal-20-solid" class="text-primary-foreground" />
          </div>
          <div>
            <h1 class="text-xl font-bold text-foreground">QuakeTrace</h1>
            <p class="text-xs text-muted-foreground">IoT Dashboard</p>
          </div>
        </div>

        <div class="flex gap-2">
        <button
          @click="$emit('update-tab', 'dashboard')"
          :class="[
            'px-4 py-2 font-medium transition-colors border-b-2',
            activeTab === 'dashboard'
              ? 'border-primary text-foreground'
              : 'border-transparent text-muted-foreground hover:text-foreground'
          ]"
        >
          Dashboard
        </button>
        <button
          @click="$emit('update-tab', 'about')"
          :class="[
            'px-4 py-2 font-medium transition-colors border-b-2',
            activeTab === 'about'
              ? 'border-primary text-foreground'
              : 'border-transparent text-muted-foreground hover:text-foreground'
          ]"
        >
          About Us
        </button>
      </div>
        
        <div class="flex items-center gap-6">
          <div class="text-right hidden sm:block">
            <p class="text-sm font-medium text-foreground">{{ currentDate }}</p>
            <p class="text-xs text-muted-foreground">{{ currentTime }}</p>
          </div>
        </div>
      </div>
    </div>
  </header>
</template>

<script setup lang="ts">
import { ref, onMounted, onUnmounted } from 'vue'

defineProps<{
  activeTab: 'dashboard' | 'about'
}>()

defineEmits<{
  'update-tab': [tab: 'dashboard' | 'about']
}>()

const currentDate = ref('')
const currentTime = ref('')

const updateDateTime = () => {
  const now = new Date()
  currentDate.value = now.toLocaleDateString('en-US', { 
    weekday: 'short', 
    month: 'short', 
    day: 'numeric',
    year: 'numeric'
  })
  currentTime.value = now.toLocaleTimeString('en-US', { 
    hour: '2-digit',
    minute: '2-digit',
    second: '2-digit',
    hour12: true
  })
}

onMounted(() => {
  updateDateTime()
  const interval = setInterval(updateDateTime, 1000)
  
  onUnmounted(() => {
    clearInterval(interval)
  })
})
</script>
