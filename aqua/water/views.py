from django.shortcuts import render

from joblib import load
model = load('./savedmodels/model.joblib')
def index(request):
    
    y_pred = " "
    if request.method == "POST":
        data = request.POST
        DO = data.get("DO")
        pH = data.get("pH")
        Temperature = data.get("Temperature")
        tds = data.get("tds")
        y_pred = model.predict([[DO,pH,Temperature,tds]])
        print(y_pred)
        print(DO,pH,Temperature,tds)
        if y_pred[0] == 1:
            y_pred = 'Healthy'
        else:
            y_pred = 'Unhealthy'   
        return render(request, "index.html", {'result' : y_pred})     
    if request.method == "GET":
        return render(request, "index.html")  
    