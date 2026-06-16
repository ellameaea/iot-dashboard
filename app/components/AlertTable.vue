<template>
  <div class="bg-card border border-border rounded-lg p-6 h-90">
    <h2 class="text-lg font-semibold text-foreground mb-6">Alert History</h2>
    
    <div class="overflow-x-auto">
      <table class="w-full">
        <thead>
          <tr class="border-b border-border">
            <th class="text-left py-3 px-4 text-sm font-semibold text-muted-foreground">Severity</th>
            <th class="text-left py-3 px-4 text-sm font-semibold text-muted-foreground">Alert</th>
            <th class="text-left py-3 px-4 text-sm font-semibold text-muted-foreground">Time</th>
            <th class="text-left py-3 px-4 text-sm font-semibold text-muted-foreground">Action</th>
          </tr>
        </thead>
        <tbody>
          <tr v-for="(alert, idx) in alerts" :key="idx" class="border-b border-border hover:bg-secondary/50 transition-colors">
            <td class="py-4 px-4">
              <span
                :class="[
                  'inline-flex items-center px-2.5 py-0.5 rounded-full text-xs font-medium',
                  alert.severity === 'critical' ? 'bg-red-900/20 text-red-300' :
                  alert.severity === 'warning' ? 'bg-amber-900/20 text-amber-300' :
                  'bg-blue-900/20 text-blue-300'
                ]"
              >
                {{ alert.severity.charAt(0).toUpperCase() + alert.severity.slice(1) }}
              </span>
            </td>
            <td class="py-4 px-4 text-sm text-foreground">{{ alert.message }}</td>
            <td class="py-4 px-4 text-sm text-muted-foreground">{{ alert.time }}</td>
            <td class="py-4 px-4">
              <button
                @click="openModal(alert)"
                class="text-primary hover:text-primary/80 text-sm font-medium transition-colors"
              >
                View
              </button>
            </td>
          </tr>
        </tbody>
      </table>
    </div>

    <!-- Modal -->
    <div
      v-if="selectedAlert"
      class="fixed inset-0 bg-black/50 flex items-center justify-center z-50 p-4"
      @click="selectedAlert = null"
    >
      <div
        class="bg-card border border-border rounded-lg p-8 max-w-md w-full"
        @click.stop
      >
        <div class="flex items-center justify-between mb-4">
          <h3 class="text-lg font-bold text-foreground">Alert Details</h3>
          <button
            @click="selectedAlert = null"
            class="text-muted-foreground hover:text-foreground transition-colors"
          >
            <UIcon name="i-heroicons-x-mark-20-solid" class="w-5 h-5" />
          </button>
        </div>

        <div class="space-y-4">
          <div>
            <p class="text-xs text-muted-foreground mb-1">Severity</p>
            <span
              :class="[
                'inline-flex items-center px-2.5 py-0.5 rounded-full text-xs font-medium',
                selectedAlert.severity === 'critical' ? 'bg-red-900/20 text-red-300' :
                selectedAlert.severity === 'warning' ? 'bg-amber-900/20 text-amber-300' :
                'bg-blue-900/20 text-blue-300'
              ]"
            >
              {{ selectedAlert.severity.charAt(0).toUpperCase() + selectedAlert.severity.slice(1) }}
            </span>
          </div>

          <div>
            <p class="text-xs text-muted-foreground mb-1">Alert Message</p>
            <p class="text-sm text-foreground">{{ selectedAlert.message }}</p>
          </div>

          <div>
            <p class="text-xs text-muted-foreground mb-1">Timestamp</p>
            <p class="text-sm text-foreground">{{ selectedAlert.timestamp }}</p>
          </div>

          <div>
            <p class="text-xs text-muted-foreground mb-1">Sensor</p>
            <p class="text-sm text-foreground">{{ selectedAlert.sensor }}</p>
          </div>

          <div>
            <p class="text-xs text-muted-foreground mb-1">Details</p>
            <p class="text-sm text-muted-foreground">{{ selectedAlert.details }}</p>
          </div>

          <button
            @click="selectedAlert = null"
            class="w-full bg-primary text-primary-foreground py-2 rounded-lg font-medium hover:opacity-90 transition-opacity"
          >
            Close
          </button>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref } from 'vue'

interface Alert {
  severity: 'critical' | 'warning' | 'info'
  message: string
  time: string
  timestamp: string
  sensor: string
  details: string
}

const alerts: Alert[] = [
  {
    severity: 'critical',
    message: 'Temperature Threshold Exceeded',
    time: '2 hours ago',
    timestamp: 'Jun 14, 2024 - 5:32 PM',
    sensor: 'Temperature Sensor - Room A',
    details: 'Temperature exceeded maximum threshold of 28°C. Current reading: 28.5°C',
  },
  {
    severity: 'warning',
    message: 'High Humidity Detected',
    time: '4 hours ago',
    timestamp: 'Jun 14, 2024 - 3:15 PM',
    sensor: 'Humidity Sensor - Room B',
    details: 'Humidity level is above recommended range (65% vs 60% max)',
  },
  {
    severity: 'warning',
    message: 'Air Quality Declining',
    time: '2 days ago',
    timestamp: 'Jun 12, 2024 - 6:45 PM',
    sensor: 'Air Quality Sensor - Room C',
    details: 'Air quality index has decreased to 52 AQI (Moderate)',
  },
]

const selectedAlert = ref<Alert | null>(null)

const openModal = (alert: Alert) => {
  selectedAlert.value = alert
}
</script>
