<template>
  <div class="grid grid-cols-1 md:grid-cols-3 gap-6">
    <div class="bg-card border border-border rounded-lg p-8 h-55">
      <div class="flex items-start justify-between mb-8">
        <div class="p-2 bg-blue-500/20 rounded-lg">
          <UIcon name="i-lucide-thermometer" class="w-5 h-5 text-blue-400" />
        </div>
        <span
          class="text-xs font-medium px-2.5 py-1 rounded-full"
          :class="temperature > 30 ? 'bg-red-500/20 text-red-400' : 'bg-green-500/20 text-green-400'"
        >
          {{ temperature > 30 ? 'Hot' : 'Normal' }}
        </span>
      </div>
      <p class="text-sm text-muted-foreground mb-1">Temperature</p>
      <div class="flex items-baseline gap-1">
        <p class="text-5xl font-bold text-foreground">{{ temperature.toFixed(1) }}</p>
        <span class="text-sm text-muted-foreground">°C</span>
      </div>
    </div>

    <div class="bg-card border border-border rounded-lg p-8 h-55">
      <div class="flex items-start justify-between mb-8">
        <div class="p-2 bg-cyan-500/20 rounded-lg">
          <UIcon name="i-lucide-wind" class="w-5 h-5 text-cyan-400" />
        </div>
        <span
          class="text-xs font-medium px-2.5 py-1 rounded-full"
          :class="moisture === 'wet' ? 'bg-red-500/20 text-red-400' : 'bg-green-500/20 text-green-400'"
        >
          {{ moisture === 'wet' ? 'Wet' : 'Dry' }}
        </span>
      </div>
      <p class="text-sm text-muted-foreground mb-1">Moisture</p>
      <div class="flex items-baseline gap-1">
        <p class="text-5xl font-bold text-foreground capitalize">{{ moisture }}</p>
      </div>
    </div>

    <div class="bg-card border border-border rounded-lg p-8 h-55">
      <div class="flex items-start justify-between mb-8">
        <div class="p-2 bg-amber-500/20 rounded-lg">
          <UIcon name="i-lucide-fan" class="w-5 h-5 text-amber-400" />
        </div>
        <span
          class="text-xs font-medium px-2.5 py-1 rounded-full"
          :class="gas > 400 ? 'bg-red-500/20 text-red-400' : 'bg-green-500/20 text-green-400'"
        >
          {{ gas > 400 ? 'Danger' : 'Normal' }}
        </span>
      </div>
      <p class="text-sm text-muted-foreground mb-1">Gas</p>
      <div class="flex items-baseline gap-1">
        <p class="text-5xl font-bold text-foreground">{{ gas }}</p>
        <span class="text-sm text-muted-foreground">ppm</span>
      </div>
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref, onMounted, onUnmounted } from 'vue'
import { createClient } from '@supabase/supabase-js'

const SUPABASE_URL = "https://kjgkzryfefbhojrqzvhh.supabase.co"
const SUPABASE_KEY = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6ImtqZ2t6cnlmZWZiaG9qcnF6dmhoIiwicm9sZSI6ImFub24iLCJpYXQiOjE3ODE0MjYwOTcsImV4cCI6MjA5NzAwMjA5N30.eDHyhA9voXdB9o3h5xlFjIhgsYc52NLsw3GWpqUaVbU"

interface SensorRow {
  id: string
  value: string        // ← TEXT now in Supabase
  updated_at: string
}

const supabase = createClient(SUPABASE_URL, SUPABASE_KEY)

const temperature = ref<number>(0.0)
const moisture    = ref<string>('dry')   // ← string now
const gas         = ref<number>(0)
const pir         = ref<boolean>(false)

defineExpose({ temperature, moisture, gas, pir })

const applyRow = (row: SensorRow) => {
  if (row.id === 'temp')     temperature.value = Number(row.value)
  if (row.id === 'moist') moisture.value    = row.value          // ← raw string, no conversion
  if (row.id === 'gas')      gas.value         = Number(row.value)
  if (row.id === 'pir')      pir.value         = Boolean(Number(row.value))
}

const fetchLatestData = async () => {
  try {
    const { data, error } = await supabase.from('live_sensors').select('*')
    if (error) { console.error('Database query failed:', error.message); return }
    if (data) (data as SensorRow[]).forEach(applyRow)
  } catch (err) {
    console.error('Snapshot connection run exception:', err)
  }
}

let realtimeChannel: any = null

const startRealtimeSync = () => {
  realtimeChannel = supabase
    .channel('card-live-stream')
    .on(
      'postgres_changes',
      { event: 'UPDATE', schema: 'public', table: 'live_sensors' },
      (payload: any) => applyRow(payload.new as SensorRow)
    )
    .subscribe()
}

onMounted(() => { fetchLatestData(); startRealtimeSync() })
onUnmounted(() => { if (realtimeChannel) supabase.removeChannel(realtimeChannel) })
</script>