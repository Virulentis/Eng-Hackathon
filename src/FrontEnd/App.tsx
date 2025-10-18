
import { ThemeProvider } from "@/components/theme-provider"
import './App.css'
import Api_call from "@/components/api"

function App() {
  

  return (
    <ThemeProvider defaultTheme="dark" 
    storageKey="vite-ui-theme">
      <Api_call></Api_call>
    
    </ThemeProvider>
  )
}

export default App