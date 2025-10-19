
import { ThemeProvider } from "@/components/theme-provider"
import './App.css'
import Api_call from "@/components/api"
import Hero from "@/components/hero.jsx"

function App() {
  

  return (
    <ThemeProvider defaultTheme="dark" 
    storageKey="vite-ui-theme">
      <Hero></Hero>
      <Api_call id="find"></Api_call>
    
    </ThemeProvider>
  )
}

export default App