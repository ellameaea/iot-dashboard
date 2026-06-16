<template>
  <div class="bg-card border border-border rounded-lg p-6 h-90">
    <h2 class="text-lg font-semibold text-foreground mb-6">Device Controls</h2>
    
    <div class="space-y-4">
      <div class="space-y-2">
        <div class="flex items-center justify-between">
          <label class="text-sm font-medium text-foreground">Heater</label>
          <div class="flex items-center gap-2">
            <span :class="['text-xs font-medium', heaterOn ? 'text-green-400' : 'text-muted-foreground']">
              {{ heaterOn ? 'ON' : 'OFF' }}
            </span>
            <button
              @click="toggleDevice('heater', heaterOn)"
              :class="[
                'relative w-12 h-6 rounded-full transition-colors',
                heaterOn ? 'bg-green-600' : 'bg-gray-600'
              ]"
            >
              <div
                :class="[
                  'absolute top-1 w-4 h-4 bg-white rounded-full transition-transform',
                  heaterOn ? 'translate-x-6' : 'translate-x-1'
                ]"
              />
            </button>
          </div>
        </div>
        <p class="text-xs text-muted-foreground">Temperature control</p>
      </div>

      <div class="space-y-2">
        <div class="flex items-center justify-between">
          <label class="text-sm font-medium text-foreground">Fan</label>
          <div class="flex items-center gap-2">
            <span :class="['text-xs font-medium', fanOn ? 'text-green-400' : 'text-muted-foreground']">
              {{ fanOn ? 'ON' : 'OFF' }}
            </span>
            <button
              @click="toggleDevice('fan', fanOn)"
              :class="[
                'relative w-12 h-6 rounded-full transition-colors',
                fanOn ? 'bg-green-600' : 'bg-gray-600'
              ]"
            >
              <div
                :class="[
                  'absolute top-1 w-4 h-4 bg-white rounded-full transition-transform',
                  fanOn ? 'translate-x-6' : 'translate-x-1'
                ]"
              />
            </button>
          </div>
        </div>
        <p class="text-xs text-muted-foreground">Circulation & cooling</p>
      </div>

      <div class="border-t border-border my-4" />

      <div class="space-y-2">
        <div class="flex items-center justify-between">
          <label class="text-sm font-medium text-foreground">Brightness</label>
          <span class="text-sm font-bold text-primary">{{ brightness }}%</span>
        </div>
        <input
          v-model.number="brightness"
          @change="updateBrightness"
          type="range"
          min="0"
          max="100"
          class="w-full h-2 bg-gray-700 rounded-lg appearance-none cursor-pointer accent-primary"
        />
        <div class="flex justify-between text-xs text-muted-foreground">
          <span>Dim</span>
          <span>Bright</span>
        </div>
      </div>

      <div class="border-t border-border my-4" />
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref, computed, onMounted, onUnmounted } from 'vue'
import { createClient } from '@supabase/supabase-js'

const SUPABASE_URL = "https://kjgkzryfefbhojrqzvhh.supabase.co"
const SUPABASE_KEY = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6ImtqZ2t6cnlmZWZiaG9qcnF6dmhoIiwicm9sZSI6ImFub24iLCJpYXQiOjE3ODE0MjYwOTcsImV4cCI6MjA5NzAwMjA5N30.eDHyhA9voXdB9o3h5xlFjIhgsYc52NLsw3GWpqUaVbU"

const supabase = createClient(SUPABASE_URL, SUPABASE_KEY)

// Reactive UI states
const heaterOn = ref<boolean>(false)
const fanOn = ref<boolean>(false)
const brightness = ref<number>(0)

const activeDevices = computed(() => {
  let count = 0
  if (heaterOn.value) count++
  if (fanOn.value) count++
  return count
})

// 1. Snapshot Initializer: Read the actual states from the database on page load
const fetchControlStates = async () => {
  const { data } = await supabase.from('device_controls').select('*')
  if (data) {
    data.forEach((row) => {
      if (row.id === 'heater') heaterOn.value = row.is_on
      if (row.id === 'fan') fanOn.value = row.is_on
      if (row.id === 'brightness') brightness.value = row.value
    })
  }
}

// 2. Action Mutators: Push switch state changes instantly to Supabase when clicked
const toggleDevice = async (deviceId: string, currentStatus: boolean) => {

  await supabase
    .from('device_controls')
    .update({ is_on: !currentStatus, updated_at: new Date().toISOString() })
    .eq('id', deviceId)
}

// Push slider values to database when dragged
const updateBrightness = async () => {
  await supabase
    .from('device_controls')
    .update({ value: brightness.value, updated_at: new Date().toISOString() })
    .eq('id', 'brightness')
}

// 3. Realtime Listener: Updates dashboard if states are changed from outside (e.g., hardware auto-shutoff)
let controlsChannel: any = null

const startControlsSync = () => {
  controlsChannel = supabase
    .channel('live-controls-stream')
    .on(
      'postgres_changes',
      { event: 'UPDATE', schema: 'public', table: 'device_controls' },
      (payload: any) => {
        const updatedRow = payload.new
        if (updatedRow.id === 'heater') heaterOn.value = updatedRow.is_on
        if (updatedRow.id === 'fan') fanOn.value = updatedRow.is_on
        if (updatedRow.id === 'brightness') brightness.value = updatedRow.value
      }
    )
    .subscribe()
}

onMounted(() => {
  fetchControlStates()
  startControlsSync()
})

onUnmounted(() => {
  if (controlsChannel) supabase.removeChannel(controlsChannel)
})
</script>