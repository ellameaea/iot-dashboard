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
const SUPABASE_KEY = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6ImtqZ2t6cnlmZWZiaG9qcnF6dmhoIiwicm9sZSI6ImFub24iLCJpYXQiOjE3ODE0MjYwOTcsImV4cCI6MjA5NzAwMjA5N30.eDHyhA9voXdB9o3h5xlFjIhgsYc52NLsw3GWpqUaVbU"

const supabase = createClient(SUPABASE_URL, SUPABASE_KEY)
const isLoading = ref<boolean>(true)

const chartData = shallowRef<any>({ labels: [], datasets: [] })

// Convert moisture string to plottable number
const moistureToNum = (val: string) => val?.toLowerCase() === 'wet' ? 1 : 0

const chartOptions = {
  responsive: true,
  maintainAspectRatio: false,
  animation: false,
  scales: {
    y: {
      grid: { color: 'rgba(255, 255, 255, 0.05)' },
      ticks: { color: '#94a3b8' }
    },
    x: {
      grid: { display: false },
      ticks: { display: false }
    }
  },
  plugins: {
    legend: { labels: { color: '#f8fafc', font: { size: 12 } } },
    tooltip: {
      callbacks: {
        label: (ctx: any) => {
          if (ctx.dataset.label.includes('Moisture')) {
            return ` Moisture: ${ctx.raw === 1 ? 'Wet' : 'Dry'}`
          }
          return ` ${ctx.dataset.label}: ${ctx.raw}`
        }
      }
    }
  }
} as any

const emptyDatasets = () => [
  {
    label: 'Temperature (°C)',
    data: [] as number[],
    borderColor: '#3b82f6',
    backgroundColor: 'rgba(59, 130, 246, 0.1)',
    tension: 0.2,
    pointRadius: 2,
    fill: true
  },
  {
    label: 'Gas (ppm)',
    data: [] as number[],
    borderColor: '#f97316',
    backgroundColor: 'rgba(249, 115, 22, 0.1)',
    tension: 0.2,
    pointRadius: 2,
    fill: true
  },
  {
    label: 'Moisture (0=Dry, 1=Wet)',
    data: [] as number[],
    borderColor: '#06b6d4',
    backgroundColor: 'rgba(6, 182, 212, 0.1)',
    tension: 0.2,
    pointRadius: 2,
    fill: true
  }
]

const fetchHistoricalLogs = async () => {
  try {
    const { data, error } = await supabase
      .from('sensor_logs')
      .select('*')
      .order('created_at', { ascending: true })
      .limit(50)

    if (error) { console.error('Historical logs fetch failed:', error.message); return }

    if (data && data.length > 0) {
      const labels: string[] = []
      const tempData: number[] = []
      const gasData: number[] = []
      const moistData: number[] = []

      // Group by timestamp — each row is one sensor reading
      data.forEach((row: any) => {
        const time = new Date(row.created_at).toLocaleTimeString([], { hour: '2-digit', minute: '2-digit' })

        if (row.topic === 'sensors/temperature') {
          labels.push(time)
          tempData.push(Number(row.value))
          gasData.push(gasData[gasData.length - 1] ?? 0)
          moistData.push(moistData[moistData.length - 1] ?? 0)
        } else if (row.topic === 'sensors/gas') {
          labels.push(time)
          gasData.push(Number(row.value))
          tempData.push(tempData[tempData.length - 1] ?? 0)
          moistData.push(moistData[moistData.length - 1] ?? 0)
        } else if (row.topic === 'sensors/moisture') {
          labels.push(time)
          moistData.push(moistureToNum(row.value))
          tempData.push(tempData[tempData.length - 1] ?? 0)
          gasData.push(gasData[gasData.length - 1] ?? 0)
        }
      })

      const ds = emptyDatasets()
      ds[0].data = tempData
      ds[1].data = gasData
      ds[2].data = moistData

      chartData.value = { labels, datasets: ds }
    }
  } catch (err) {
    console.error('Chart fetch error:', err)
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
        const d = new Date(newLog.created_at)
        const timeLabel = d.toLocaleTimeString([], { hour: '2-digit', minute: '2-digit' })

        const nextLabels  = [...chartData.value.labels]
        const nextTemp    = [...chartData.value.datasets[0].data]
        const nextGas     = [...chartData.value.datasets[1].data]
        const nextMoist   = [...chartData.value.datasets[2].data]

        nextLabels.push(timeLabel)

        if (newLog.topic === 'sensors/temperature') {
          nextTemp.push(Number(newLog.value))
          nextGas.push(nextGas[nextGas.length - 1] ?? 0)
          nextMoist.push(nextMoist[nextMoist.length - 1] ?? 0)
        } else if (newLog.topic === 'sensors/gas') {
          nextGas.push(Number(newLog.value))
          nextTemp.push(nextTemp[nextTemp.length - 1] ?? 0)
          nextMoist.push(nextMoist[nextMoist.length - 1] ?? 0)
        } else if (newLog.topic === 'sensors/moisture') {
          nextMoist.push(moistureToNum(newLog.value))
          nextTemp.push(nextTemp[nextTemp.length - 1] ?? 0)
          nextGas.push(nextGas[nextGas.length - 1] ?? 0)
        }

        // Cap at 30 points
        if (nextLabels.length > 30) {
          nextLabels.shift()
          nextTemp.shift()
          nextGas.shift()
          nextMoist.shift()
        }

        chartData.value = {
          labels: nextLabels,
          datasets: [
            { ...chartData.value.datasets[0], data: nextTemp  },
            { ...chartData.value.datasets[1], data: nextGas   },
            { ...chartData.value.datasets[2], data: nextMoist },
          ]
        }
      }
    )
    .subscribe()
}

onMounted(() => { fetchHistoricalLogs(); startLiveGraphSync() })
onUnmounted(() => { if (logsChannel) supabase.removeChannel(logsChannel) })
</script>