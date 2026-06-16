<template>
  <div class="min-h-screen bg-background text-foreground">
    <DashboardHeader :active-tab="activeTab" @update-tab="activeTab = $event" />

    <main v-if="activeTab === 'dashboard'" class="p-6 space-y-6 max-w-full mx-auto pb-0">

      <!-- Room Selector Bar -->
      <div
        class="flex items-center justify-between bg-card rounded-lg px-5 py-3"
        :style="{ border: `1.5px solid ${roomBorderColor}` }"
      >
        <div class="flex items-center gap-2 text-sm font-medium text-foreground">
          <svg xmlns="http://www.w3.org/2000/svg" class="w-4 h-4 text-muted-foreground" viewBox="0 0 24 24"
            fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
            <rect x="3" y="3" width="7" height="7"/><rect x="14" y="3" width="7" height="7"/>
            <rect x="3" y="14" width="7" height="7"/><rect x="14" y="14" width="7" height="7"/>
          </svg>
          Viewing:
        </div>
        <div class="relative">
          <select
            v-model="selectedRoom"
            class="appearance-none bg-muted border border-border rounded-md pl-3 pr-8 py-1.5 text-sm font-medium text-foreground cursor-pointer focus:outline-none focus:ring-2 focus:ring-ring"
          >
            <option value="a">Room A</option>
            <option value="b">Room B</option>
            <option value="c">Room C</option>
          </select>
          <svg xmlns="http://www.w3.org/2000/svg"
            class="pointer-events-none absolute right-2 top-1/2 -translate-y-1/2 w-3.5 h-3.5 text-muted-foreground"
            viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"
            stroke-linecap="round" stroke-linejoin="round">
            <polyline points="6 9 12 15 18 9"/>
          </svg>
        </div>
      </div>

      <div class="flex gap-6">
        <!-- Left 60% -->
        <div class="w-[60%] space-y-6">
          <AlertBanner />
          <SensorCards ref="sensorCards" />
          <!-- <AlertTable /> -->
        </div>

        <!-- Right 40% -->
        <div class="w-[40%] space-y-6">
          <SafetyView
            :temperature="sensorCards?.temperature ?? 0"
            :moisture="sensorCards?.moisture ?? 'dry'"
            :gas="sensorCards?.gas ?? 0"
            :pir="sensorCards?.pir ?? false"
          />
        </div>
      </div>
       <TimeSeriesChart />
    </main>

    <main v-else class="p-6 max-w-4xl mx-auto">
      <div class="bg-card rounded-lg p-8 border border-border">
        <h2 class="text-3xl font-bold mb-4">About QuakeTrace</h2>
        <p class="text-muted-foreground mb-4">
          The project focuses on post-earthquake disaster preparedness and emergency response. The system DOES NOT detect earthquakes directly. 
          Instead, it monitors secondary hazards commonly encountered after earthquakes such as gas leaks, smoke hazards, water leaks, unsafe environmental conditions, and possible trapped occupants.
        </p>
        <div class="space-y-4 text-muted-foreground">
          <p><strong class="text-foreground">Real-time Monitoring:</strong> Track multiple sensors in real-time.</p>
          <p><strong class="text-foreground">Alert Management:</strong> Stay informed with instant alerts when sensors exceed thresholds.</p>
        </div>
      </div>
    </main>
  </div>
</template>

<script setup lang="ts">
import { ref, computed } from 'vue'
import SensorCards from '../components/SensorCards.vue'
import SafetyView from '../components/SafetyView.vue'
import AlertBanner from '../components/AlertBanner.vue'
import AlertTable from '../components/AlertTable.vue'
import DashboardHeader from '../components/DashboardHeader.vue'
import TimeSeriesChart from '../components/TimeSeriesChart.vue'

const activeTab = ref<'dashboard' | 'about'>('dashboard')
const selectedRoom = ref<'a' | 'b' | 'c'>('a')
const sensorCards = ref<InstanceType<typeof SensorCards> | null>(null)

const roomBorderColor = computed(() => ({
  a: '#ef4444',
  b: '#f97316',
  c: '#eab308',
}[selectedRoom.value]))
</script>