<template>
  <div class="w-full h-[96px] mb-6 overflow-hidden flex flex-col justify-center">
    
    <div 
      v-if="isAlertActive && showBanner" 
      class="w-full h-full bg-red-900/20 border border-red-800 text-red-200 px-6 py-4 rounded-lg flex items-center justify-between"
    >
      <div class="flex items-center gap-3 min-w-0 h-full">
        <div class="p-2 bg-red-500/20 rounded-lg flex-shrink-0 animate-pulse flex items-center justify-center">
          <UIcon name="i-heroicons-exclamation-triangle-20-solid" class="w-5 h-5 text-red-500" />
        </div>
        
        <div class="min-w-0 h-full flex flex-col justify-center">
          <p class="font-semibold text-red-400 text-sm md:text-base leading-none mb-1 truncate">
            Critical Environmental Alert
          </p>
          
          <div class="text-xs md:text-sm text-red-300 space-y-0.5">
            <p v-if="temperature > 30" class="truncate leading-tight">
              Temperature is <strong class="text-red-400">Hot</strong>: {{ temperature.toFixed(1) }}°C (Threshold: > 30°C)
            </p>
            <p v-if="moisture === 'wet'" class="truncate leading-tight">
              Moisture is <strong class="text-red-400">Wet</strong>: Soil moisture threshold exceeded
            </p>
            <p v-if="gas > 400" class="truncate leading-tight">
              Gas level is <strong class="text-red-400">Elevated</strong>: {{ gas }} ppm (Threshold: > 400 ppm)
            </p>
            <p v-if="pir" class="truncate leading-tight">
              Occupancy <strong class="text-red-400">Detected</strong>: Propable person
            </p>
          </div>
        </div>
      </div>
    </div>

    <div 
      v-else-if="!isAlertActive" 
      class="w-full h-full bg-green-950/20 border border-green-800/60 text-green-200 px-6 py-4 rounded-lg flex items-center justify-between"
    >
      <div class="flex items-center gap-3 h-full">
        <div class="p-2 bg-green-500/20 rounded-lg flex-shrink-0 flex items-center justify-center">
          <UIcon name="i-heroicons-check-circle-20-solid" class="w-5 h-5 text-green-400" />
        </div>
        <div class="flex flex-col justify-center h-full">
          <p class="font-semibold text-green-400 text-sm md:text-base leading-none mb-1">Normal System</p>
          <p class="text-xs md:text-sm text-green-300/90 leading-tight">No alert — All telemetry metrics are within safe operational limits.</p>
        </div>
      </div>
    </div>

  </div>
</template>

<script setup lang="ts">
import { ref, computed, onMounted, onUnmounted } from 'vue'
import { createClient } from '@supabase/supabase-js'

const SUPABASE_URL = "https://kjgkzryfefbhojrqzvhh.supabase.co"
const SUPABASE_KEY = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6ImtqZ2t6cnlmZWZiaG9qcnF6dmhoIiwicm9sZSI6ImFub24iLCJpYXQiOjE3ODE0MjYwOTcsImV4cCI6MjA5NzAwMjA5N30.eDHyhA9voXdB9o3h5xlFjIhgsYc52NLsw3GWpqUaVbU"

const supabase = createClient(SUPABASE_URL, SUPABASE_KEY)

interface SensorRow {
  id: string
  value: string   // TEXT in Supabase now
}

const temperature = ref<number>(0.0)
const moisture    = ref<string>('dry')
const gas         = ref<number>(0)
const pir         = ref<boolean>(false)
const showBanner  = ref<boolean>(true)

const isAlertActive = computed(() => {
  return temperature.value > 30 || moisture.value === 'wet' || gas.value > 400 || pir.value
})

const applyRow = (row: SensorRow) => {
  if (row.id === 'temp')     temperature.value = Number(row.value)
  if (row.id === 'moist') moisture.value    = row.value.toLowerCase()
  if (row.id === 'gas')      gas.value         = Number(row.value)
  if (row.id === 'pir')      pir.value         = Boolean(Number(row.value))
}

const fetchCurrentThresholds = async () => {
  try {
    const { data, error } = await supabase.from('live_sensors').select('*')
    if (error) return
    if (data) (data as SensorRow[]).forEach(applyRow)
  } catch (err) {
    console.error('Alert engine snapshot failure:', err)
  }
}

let alertChannel: any = null

const startAlertSync = () => {
  alertChannel = supabase
    .channel('alert-live-stream')
    .on(
      'postgres_changes',
      { event: 'UPDATE', schema: 'public', table: 'live_sensors' },
      (payload: any) => {
        const row = payload.new as SensorRow

        if (row.id === 'temp') {
          const wasAlert = temperature.value > 30
          temperature.value = Number(row.value)
          if (wasAlert && temperature.value <= 30) showBanner.value = true
        }

        if (row.id === 'moisture') {
          const wasAlert = moisture.value === 'wet'
          moisture.value = row.value.toLowerCase()
          if (wasAlert && moisture.value !== 'wet') showBanner.value = true
        }

        if (row.id === 'gas') {
          const wasAlert = gas.value > 400
          gas.value = Number(row.value)
          if (wasAlert && gas.value <= 400) showBanner.value = true
        }

        if (row.id === 'pir') {
          pir.value = Boolean(Number(row.value))
        }
      }
    )
    .subscribe()
}

onMounted(() => {
  fetchCurrentThresholds()
  startAlertSync()
})

onUnmounted(() => {
  if (alertChannel) supabase.removeChannel(alertChannel)
})
</script>