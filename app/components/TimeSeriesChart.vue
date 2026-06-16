<template>
  <div class="bg-card border border-border rounded-lg p-6">
    <h2 class="text-lg font-semibold text-foreground mb-6">Sensor Data (Historical Logs)</h2>
    <div style="width: 100%; height: 240px;" class="relative">
      <div v-if="isLoading" class="absolute inset-0 flex items-center justify-center bg-card/50 text-sm text-muted-foreground">
        Loading historical trends...
      </div>
      <Line v-if="!isLoading" :data="chartData" :options="chartOptions" />
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref, shallowRef, onMounted, onUnmounted } from 'vue'
import { Line } from 'vue-chartjs'
import { createClient } from '@supabase/supabase-js'
import {
  Chart as ChartJS,
  CategoryScale,
  LinearScale,
  PointElement,
  LineElement,
  Title,
  Tooltip,
  Legend,
  Filler
} from 'chart.js'

ChartJS.register(CategoryScale, LinearScale, PointElement, LineElement, Title, Tooltip, Legend, Filler)

const SUPABASE_URL = "https://kjgkzryfefbhojrqzvhh.supabase.co"
const SUPABASE_KEY = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6ImtqZ2t6cnlmZWZiaG9qcnF6dmhoIiwicm9sZSI6ImFub24iLCJpYXQiOjE3ODE0MjYwOTcsImV4cCI6MjA5NzAwMjA5N30.eDHyhA9voXdB9o3h5xlFjIhgsYc52NLsw3GWpqUaVbU" // <-- Paste your actual key here

const supabase = createClient(SUPABASE_URL, SUPABASE_KEY)
const isLoading = ref<boolean>(true)

// FIX 1: Use shallowRef instead of ref to stop Vue's deep proxy memory leak loop!
const chartData = shallowRef<any>({ labels: [], datasets: [] })

const chartOptions = {
  responsive: true,
  maintainAspectRatio: false,
  animation: false, // Turn off heavy animations for quick real-time ticks
  scales: {
    y: {
      grid: { color: 'rgba(255, 255, 255, 0.05)' },
      ticks: { color: '#94a3b8' }
    },
    x: {
      grid: { display: false },
      ticks: { display: false }
      // ticks: { color: '#94a3b8', maxTicksLimit: 8 }
    }
  },
  plugins: {
    legend: { labels: { color: '#f8fafc', font: { size: 12 } } }
  }
} as any

// Historical logs initial fetch
const fetchHistoricalLogs = async () => {
  try {
    const { data, error } = await supabase
      .from('sensor_logs')
      .select('*')
      .order('created_at', { ascending: true })
      .limit(50)

    if (error) {
      console.error('Historical logs data table fetch failed:', error.message)
      return
    }

    if (data && data.length > 0) {
      const times = data.map((row: any) => {
        const d = new Date(row.created_at)
        return d.toLocaleTimeString([], { hour: '2-digit', minute: '2-digit' })
      })

      // Adjusting data mapping logic to handle your table's single-value topic structure
      const tempValues = data.filter((row: any) => row.topic === 'sensors/temp' || row.id === 'temperature').map((row: any) => row.value)
      const humValues = data.filter((row: any) => row.topic === 'sensors/humidity' || row.id === 'humidity').map((row: any) => row.value)

      chartData.value = {
        labels: times.slice(0, Math.max(tempValues.length, humValues.length)),
        datasets: [
          {
            label: 'Temperature (°C)',
            data: tempValues,
            borderColor: '#3b82f6',
            backgroundColor: 'rgba(59, 130, 246, 0.1)',
            tension: 0.2,
            pointRadius: 2,
            fill: true
          },
          {
            label: 'Humidity (%)',
            data: humValues,
            borderColor: '#06b6d4',
            backgroundColor: 'rgba(6, 182, 212, 0.1)',
            tension: 0.2,
            pointRadius: 2,
            fill: true
          }
        ]
      }
    }
  } catch (err) {
    console.error('Chart tracking connection run error:', err)
  } finally {
    isLoading.value = false
  }
}

let logsChannel: any = null

const startLiveGraphSync = () => {
  logsChannel = supabase
    .channel('chart-live-stream')
    .on(
      'postgres_changes',
      { event: 'INSERT', schema: 'public', table: 'sensor_logs' },
      (payload: any) => {
        const newLog = payload.new
        console.log('New database logging point appended to graph views:', newLog)
        
        const d = new Date(newLog.created_at)
        const timeLabel = d.toLocaleTimeString([], { hour: '2-digit', minute: '2-digit' })
        
        // Clone the arrays safely outside of reactive proxies
        const nextLabels = [...chartData.value.labels]
        const nextTempData = [...chartData.value.datasets[0].data]
        const nextHumData = [...chartData.value.datasets[1].data]

        // Add timestamps to labels
        nextLabels.push(timeLabel)

        // Parse metrics dynamically depending on the telemetry topic line structure
        if (newLog.topic === 'sensors/temp' || newLog.id === 'temperature') {
          nextTempData.push(Number(newLog.value))
          // Pad the opposite array so our dataset arrays keep perfectly matching lengths
          if (nextHumData.length < nextTempData.length) {
            nextHumData.push(nextHumData[nextHumData.length - 1] || 0)
          }
        } else if (newLog.topic === 'sensors/humidity' || newLog.id === 'humidity') {
          nextHumData.push(Number(newLog.value))
          if (nextTempData.length < nextHumData.length) {
            nextTempData.push(nextTempData[nextTempData.length - 1] || 0)
          }
        }

        // Cap array size limits cleanly
        if (nextLabels.length > 30) {
          nextLabels.shift()
          nextTempData.shift()
          nextHumData.shift()
        }

        // FIX 2: Trigger root-level reassignment so shallowRef fires a clean UI redraw!
        chartData.value = {
          labels: nextLabels,
          datasets: [
            { ...chartData.value.datasets[0], data: nextTempData },
            { ...chartData.value.datasets[1], data: nextHumData }
          ]
        }
      }
    )
    .subscribe()
}

onMounted(() => {
  fetchHistoricalLogs()
  startLiveGraphSync()
})

onUnmounted(() => {
  if (logsChannel) {
    supabase.removeChannel(logsChannel)
  }
})
</script>