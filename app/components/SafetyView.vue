<template>
  <div class="text-foreground">
    <!-- <div class="max-w-4xl mx-auto grid grid-cols-2 gap-4"> -->
    <div class="grid grid-cols-2 gap-4 items-stretch">
      <!-- LEFT: Room Safety Index -->
      <div class="bg-card border border-border rounded-xl p-6 space-y-6">
        <h2 class="text-xs font-semibold tracking-widest text-muted-foreground uppercase">
          Room Safety Index
        </h2>

        <!-- Gauge -->
        <div class="flex flex-col items-center gap-2">
          <div class="relative w-40 h-20 overflow-hidden">
            <svg viewBox="0 0 160 80" class="w-full h-full">
              <path
                d="M 10 80 A 70 70 0 0 1 150 80"
                fill="none"
                stroke="currentColor"
                stroke-width="12"
                stroke-linecap="round"
                class="text-muted/30"
              />
              <path
                d="M 10 80 A 70 70 0 0 1 150 80"
                fill="none"
                :stroke="gaugeColor"
                stroke-width="12"
                stroke-linecap="round"
                :stroke-dasharray="safetyArc"
                stroke-dashoffset="0"
              />
            </svg>
            <div class="absolute inset-0 flex flex-col items-center justify-end pb-1">
              <span class="text-3xl font-bold text-foreground">{{ criticalCount }}</span>
              <span class="text-xs text-muted-foreground">/ 4</span>
            </div>
          </div>

          <span
            class="px-6 py-1 rounded text-sm font-semibold tracking-wide"
            :class="statusClass"
          >
            {{ statusLabel }}
          </span>
        </div>

        <!-- Sensor breakdown -->
        <div class="space-y-3">
          <div v-for="sensor in sensors" :key="sensor.label" class="space-y-1">
            <div class="flex justify-between text-sm">
              <span class="text-muted-foreground">{{ sensor.label }}</span>
              <span :class="sensor.critical ? 'text-red-400 font-semibold' : 'text-foreground font-medium'">
                {{ sensor.critical ? 'Critical' : 'Normal' }}
              </span>
            </div>
            <div class="h-1.5 rounded-full bg-muted overflow-hidden">
              <div
                class="h-full rounded-full transition-all duration-500"
                :style="{ width: sensor.critical ? '100%' : '20%', backgroundColor: sensor.critical ? '#ef4444' : '#6366f1' }"
              />
            </div>
          </div>
        </div>
      </div>

      <!-- RIGHT: Occupancy (PIR) -->
      <!-- RIGHT: Occupancy (PIR) -->
<div class="h-full">
  <div class="bg-card border border-border rounded-xl p-6 h-full flex flex-col">
    <h2 class="text-xs font-semibold tracking-widest text-muted-foreground uppercase">
      Occupancy (PIR)
    </h2>

    <div class="flex-1 flex flex-col items-center justify-center gap-2">
  <svg xmlns="http://www.w3.org/2000/svg"
    class="w-14 h-14 transition-colors duration-300"
    :class="pir ? 'text-red-400' : 'text-green-400'"
    viewBox="0 0 24 24"
    fill="none" stroke="currentColor" stroke-width="1.25" stroke-linecap="round" stroke-linejoin="round">
    <circle cx="12" cy="7" r="4"/>
    <path d="M5.5 21a8.38 8.38 0 0 1 13 0"/>
  </svg>
  <p class="text-base font-semibold text-foreground">
    {{ pir ? 'Occupied' : 'Unoccupied' }}
  </p>
  <p class="text-xs text-muted-foreground">
    {{ pir ? 'Motion detected' : 'No motion detected' }}
  </p>
</div>

<div class="space-y-2">
  <p class="text-xs text-muted-foreground">LED Status Indicators</p>
  <div class="flex gap-2">
    <span
      class="w-4 h-4 rounded-full border border-black/10 transition-colors duration-300"
      :style="{ backgroundColor: pir ? '#ef4444' : '#374151' }"
    />
    <span
      class="w-4 h-4 rounded-full border border-black/10 transition-colors duration-300"
      :style="{ backgroundColor: !pir ? '#22c55e' : '#374151' }"
    />
  </div>
</div>
  </div>
</div>

    </div>
  </div>
</template>

<script setup lang="ts">
import { computed } from 'vue'

const props = defineProps<{
  temperature: number
  moisture: string
  gas: number
  pir: boolean
}>()

const sensors = computed(() => [
  { label: 'Temperature', critical: props.temperature > 30 },
  { label: 'Moisture',    critical: props.moisture === 'wet'    },
  { label: 'Gas/Smoke',   critical: props.gas > 400        },
  { label: 'Occupancy',   critical: props.pir              },
])

const criticalCount = computed(() => sensors.value.filter(s => s.critical).length)

const statusLabel = computed(() => {
  if (criticalCount.value === 0) return 'SAFE'
  if (criticalCount.value <= 2)  return 'WARNING'
  return 'CRITICAL'
})

const statusClass = computed(() => ({
  'SAFE':     'bg-green-500/20 text-green-400',
  'WARNING':  'bg-yellow-500/20 text-yellow-400',
  'CRITICAL': 'bg-red-500/20 text-red-400',
}[statusLabel.value]))

const gaugeColor = computed(() => ({
  'SAFE':     '#22c55e',
  'WARNING':  '#eab308',
  'CRITICAL': '#ef4444',
}[statusLabel.value]))

const safetyArc = computed(() => {
  const total = 220
  const filled = (criticalCount.value / 4) * total
  return `${filled} ${total}`
})

const leds = computed(() => [
  { id: 'green',  color: '#22c55e', active: statusLabel.value === 'SAFE'     },
  { id: 'yellow', color: '#eab308', active: statusLabel.value === 'WARNING'  },
  { id: 'red',    color: '#ef4444', active: statusLabel.value === 'CRITICAL' },
])
</script>