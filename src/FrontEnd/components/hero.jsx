import { Button } from "@/components/ui/button"
import './hero.css'

export default function Hero() {
  return (
    
    <div className="bg-[url('./public/Assignments.jpg')] bg-cover bg-center min-h-screen flex items-center justify-center "  id="Hero">
      <div className="text-center animate-fade-in text-white px-2 max-w-2xl">
        <h1 className="font-['Permanent_Marker'] fa-map-marker-alt translate-x-20 text-6xl text-red-400 font-bold mb-6 "> 
              Assignment Checker
        </h1>
        <p className="font-['Permanent_Marker'] text-xl text-blue-400 mb-8">
             Search for your assignments!
        </p>
        
      </div>
    </div>
  )
}