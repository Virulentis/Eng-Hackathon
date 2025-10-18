import { Button } from "@/components/ui/button"


export default function Hero() {
  return (
    <div className="min-h-screen flex items-center justify-center " id="Hero">
      <div className="text-center animate-fade-in text-white px-2 max-w-2xl">
        <h1 className="text-6xl font-bold mb-6 "> 
          Assignment Checker
        </h1>
        <p className="text-xl text-slate-300 mb-8">
          Search for your assignments!
        </p>
        <Button size="lg" variant="default">
            <a href="#find">Find Assignments</a>
        </Button>
        
      </div>
    </div>
  )
}